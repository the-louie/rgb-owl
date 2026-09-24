#include "ble.h"

#include <NimBLEDevice.h>
#include <WiFi.h>

#include "app.h"
#include "effect.h"
#include "log.h"
#include "net.h"
#include "clock.h"
#include "config.h"
#include "crash.h"
#include "owl/pairing.h"
#include "owl/protocol.h"
#include "update.h"

namespace owl::ble {

static_assert(OWL_BLE_PIN >= 0 && OWL_BLE_PIN <= 999999, "ble_pin in secrets.ini must be 6 digits");

constexpr uint32_t AUTH_TIMEOUT_MS = 30000;  // unauthenticated links are dropped after this
constexpr uint32_t STATE_POLL_MS = 100;
constexpr int MAX_LINKS = 3;

struct Link {
    uint16_t handle;
    uint32_t since;
    bool authed;
    bool used;
};

static NimBLEServer* server;
static NimBLECharacteristic* stateChr;
static NimBLECharacteristic* eventChr;
static QueueHandle_t commands;  // char[Command::MAX_LEN + 1], filled by the NimBLE task
static Link links[MAX_LINKS];
static portMUX_TYPE linksMux = portMUX_INITIALIZER_UNLOCKED;
static String lastState;
static uint32_t lastPoll;

static bool isAuthed(uint16_t handle) {
    bool ok = false;
    portENTER_CRITICAL(&linksMux);
    for (auto& l : links)
        if (l.used && l.handle == handle) ok = l.authed;
    portEXIT_CRITICAL(&linksMux);
    return ok;
}

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer*, ble_gap_conn_desc* desc) override {
        bool bonded = NimBLEDevice::isBonded(NimBLEAddress(desc->peer_id_addr));
        if (!pairingAllowed(bonded, millis(), config::PAIRING_WINDOW_MS)) {
            server->disconnect(desc->conn_handle);  // unknown phone after the pairing window
            return;
        }
        portENTER_CRITICAL(&linksMux);
        for (auto& l : links)
            if (!l.used) {
                l = {desc->conn_handle, millis(), false, true};
                break;
            }
        portEXIT_CRITICAL(&linksMux);
        NimBLEDevice::startSecurity(desc->conn_handle);  // pair / re-encrypt right away
    }
    void onDisconnect(NimBLEServer*, ble_gap_conn_desc* desc) override {
        portENTER_CRITICAL(&linksMux);
        for (auto& l : links)
            if (l.used && l.handle == desc->conn_handle) l.used = false;
        portEXIT_CRITICAL(&linksMux);
    }
    uint32_t onPassKeyRequest() override { return OWL_BLE_PIN; }
    void onAuthenticationComplete(ble_gap_conn_desc* desc) override {
        bool ok = desc->sec_state.encrypted && desc->sec_state.authenticated;
        portENTER_CRITICAL(&linksMux);
        for (auto& l : links)
            if (l.used && l.handle == desc->conn_handle) l.authed = ok;
        portEXIT_CRITICAL(&linksMux);
        if (!ok) server->disconnect(desc->conn_handle);
    }
};

class CommandCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* c, ble_gap_conn_desc* desc) override {
        if (!isAuthed(desc->conn_handle)) return;  // belt and braces: WRITE_AUTHEN already enforces it
        std::string v = c->getValue();
        char line[Command::MAX_LEN + 1] = {};
        memcpy(line, v.data(), v.size() < Command::MAX_LEN ? v.size() : Command::MAX_LEN);
        xQueueSend(commands, line, 0);  // full queue: command dropped, app retries
    }
};

static void sendEvent(const char* json) {
    if (net::devmode()) log::printf("event: %s", json);
    eventChr->setValue(reinterpret_cast<const uint8_t*>(json), strlen(json));
    eventChr->notify();
}

static void sendResult(const char* verb, const char* error) {
    char buf[200];
    JsonWriter w(buf, sizeof(buf));
    w.str("type", error ? "error" : "ok").str("verb", verb);
    if (error) w.str("msg", error);
    if (const char* j = w.finish()) sendEvent(j);
}

static String stateJson() {
    char buf[256];
    size_t n = toJson(buf, sizeof(buf) - 40, app::settings(), app::currentEffect());
    if (!n) return "{}";
    String s(buf);
    s.remove(s.length() - 1);  // reopen the object to append more fields
    s += ",\"wifi\":\"";
    s += net::statusName();
    s += "\",\"devmode\":";
    s += net::devmode() ? "true" : "false";
    s += ",\"version\":\"" OWL_VERSION "\"}";
    if (s.length() > 240) log::printf("ble: state is %u bytes, notifications cut at MTU-3", s.length());
    return s;
}

static void pushState(bool force) {
    String s = stateJson();
    if (!force && s == lastState) return;
    lastState = s;
    stateChr->setValue(reinterpret_cast<const uint8_t*>(s.c_str()), s.length());
    stateChr->notify();
}

static void handle(const char* line) {
    Command cmd;
    if (!cmd.parse(line)) return sendResult("?", "malformed command");
    const char* verb = cmd.verb();
    if (!strcmp(verb, "get")) {
        pushState(true);
        return sendResult(verb, nullptr);
    }
    if (!strcmp(verb, "set")) {
        for (size_t i = 0; i < cmd.size(); ++i) {
            String value = cmd.value(i);
            if (!strcmp(cmd.key(i), "effect")) {
                int idx = EFFECTS.find(value.c_str());
                if (idx >= 0) value = String(idx);
            }
            ApplyResult r = app::set(cmd.key(i), value.c_str());
            if (r == ApplyResult::UnknownKey) return sendResult(verb, "unknown setting");
            if (r == ApplyResult::BadValue) return sendResult(verb, "bad value");
        }
        pushState(false);
        return sendResult(verb, nullptr);
    }
    if (!strcmp(verb, "config")) {  // settings that do not fit the state notification
        char buf[200];
        JsonWriter w(buf, sizeof(buf));
        const Settings& s = app::settings();
        w.str("type", "config").num("cycle", long(s.cycle)).boolean("night", s.night)
            .num("night_from", s.nightFrom).num("night_to", s.nightTo).boolean("time_set", clock::valid())
            .str("tz", clock::tz().c_str());
        if (const char* j = w.finish()) sendEvent(j);
        return sendResult(verb, nullptr);
    }
    if (!strcmp(verb, "debug")) {  // two events, each under the notification size limit
        char buf[240];
        JsonWriter a(buf, sizeof(buf));
        a.str("type", "debug").num("uptime_s", long(millis() / 1000)).str("reset", crash::resetReason())
            .num("heap", long(ESP.getFreeHeap())).num("fps", long(app::fps())).num("ma", long(app::estimatedMilliamps()))
            .str("ip", net::online() ? WiFi.localIP().toString().c_str() : "");
        if (const char* j = a.finish()) sendEvent(j);
        JsonWriter b(buf, sizeof(buf));
        b.str("type", "debug").str("last_crash", crash::lastReason()).num("last_crash_time", long(crash::lastTime()))
            .num("crashes", long(crash::count())).num("rssi", net::online() ? WiFi.RSSI() : 0)
            .num("bonds", NimBLEDevice::getNumBonds());
        if (const char* j = b.finish()) sendEvent(j);
        return sendResult(verb, nullptr);
    }
    if (!strcmp(verb, "project")) {  // project [url=<GitHub URL or owner/repo>] -> project event
        if (const char* url = cmd.get("url"))
            if (!update::setProject(url)) return sendResult(verb, "not a GitHub project");
        char buf[200];
        JsonWriter w(buf, sizeof(buf));
        w.str("type", "project").str("project", update::project().c_str());
        if (const char* j = w.finish()) sendEvent(j);
        return sendResult(verb, nullptr);
    }
    if (!strcmp(verb, "test")) {  // test mode=none|off|solid|pixel|column|row|walk[&r&g&b&index]
        auto num = [&](const char* k) { const char* v = cmd.get(k); return v ? atoi(v) : 0; };
        const char* mode = cmd.get("mode");
        if (!mode || !app::setTest(mode, uint8_t(num("r")), uint8_t(num("g")), uint8_t(num("b")), num("index")))
            return sendResult(verb, "bad test mode or index");
        return sendResult(verb, nullptr);
    }
    if (!strcmp(verb, "time")) {  // time epoch=<unix seconds>&tz=<POSIX TZ>
        const char* epoch = cmd.get("epoch");
        if (!epoch || strtoul(epoch, nullptr, 10) < 1700000000UL) return sendResult(verb, "bad epoch");
        clock::set(strtoul(epoch, nullptr, 10), cmd.get("tz"));
        return sendResult(verb, nullptr);
    }
    if (!strcmp(verb, "wifi_scan")) {
        if (!net::startScan()) return sendResult(verb, "wifi busy");
        return sendResult(verb, nullptr);  // networks follow as wifi_net events
    }
    if (!strcmp(verb, "wifi_test") || !strcmp(verb, "wifi_save")) {
        const char* ssid = cmd.get("ssid");
        const char* pass = cmd.get("pass");
        if (!ssid || !pass) return sendResult(verb, "missing ssid or pass");
        if (verb[5] == 't') return sendResult(verb, net::startTest(ssid, pass) ? nullptr : "wifi busy");
        if (!net::saveTested(ssid, pass)) return sendResult(verb, "test these credentials first");
        pushState(true);
        return sendResult(verb, nullptr);
    }
    if (!strcmp(verb, "wifi_forget")) {
        net::forgetCredentials();
        pushState(true);
        return sendResult(verb, nullptr);
    }
    if (!strcmp(verb, "wifi_info")) {
        char buf[200];
        JsonWriter w(buf, sizeof(buf));
        w.str("type", "wifi_info").boolean("configured", net::configured()).str("ssid", net::ssidName().c_str())
            .str("status", net::statusName()).str("ip", net::online() ? WiFi.localIP().toString().c_str() : "");
        if (const char* j = w.finish()) sendEvent(j);
        return sendResult(verb, nullptr);
    }
    if (!strcmp(verb, "devmode")) {
        const char* on = cmd.get("on");
        if (!on) return sendResult(verb, "missing on");
        net::setDevmode(strcmp(on, "0") != 0);
        pushState(true);
        return sendResult(verb, nullptr);
    }
    sendResult(verb, "unknown verb");
}

void begin() {
    commands = xQueueCreate(4, Command::MAX_LEN + 1);
    NimBLEDevice::init("Owl");
    NimBLEDevice::setSecurityAuth(true, true, true);  // bonding, MITM, LE Secure Connections
    NimBLEDevice::setSecurityPasskey(OWL_BLE_PIN);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);  // the owl "displays" the fixed PIN
    NimBLEDevice::setMTU(247);

    server = NimBLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());
    NimBLEService* svc = server->createService(SERVICE_UUID);
    const uint32_t R = NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN;
    const uint32_t W = NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN;
    stateChr = svc->createCharacteristic(STATE_UUID, R | NIMBLE_PROPERTY::NOTIFY);
    svc->createCharacteristic(COMMAND_UUID, W)->setCallbacks(new CommandCallbacks());
    eventChr = svc->createCharacteristic(EVENT_UUID, R | NIMBLE_PROPERTY::NOTIFY);

    String names = "[";
    for (size_t i = 0; i < EFFECTS.size(); ++i) {
        if (i) names += ',';
        names += '"';
        names += EFFECTS[i].name();
        names += '"';
    }
    names += ']';
    svc->createCharacteristic(EFFECTS_UUID, R)->setValue(names.c_str());
    svc->start();
    pushState(true);
    net::setEventSink(sendEvent);

    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    adv->setScanResponse(true);
    adv->start();
    log::printf("ble: advertising as Owl, %d bonded phone(s)", NimBLEDevice::getNumBonds());
}

void execute(const char* line) { handle(line); }

int bondCount() { return NimBLEDevice::getNumBonds(); }

void loop() {
    char line[Command::MAX_LEN + 1];
    while (xQueueReceive(commands, line, 0) == pdTRUE) handle(line);

    uint32_t now = millis();
    if (now - lastPoll < STATE_POLL_MS) return;
    lastPoll = now;
    pushState(false);

    // drop links that never authenticated (e.g. a stranger without the PIN)
    uint16_t drop[MAX_LINKS];
    int n = 0;
    portENTER_CRITICAL(&linksMux);
    for (auto& l : links)
        if (l.used && !l.authed && now - l.since > AUTH_TIMEOUT_MS) drop[n++] = l.handle;
    portEXIT_CRITICAL(&linksMux);
    for (int i = 0; i < n; ++i) {
        log::printf("ble: dropping unauthenticated link %u", drop[i]);
        server->disconnect(drop[i]);
    }
}

}  // namespace owl::ble
