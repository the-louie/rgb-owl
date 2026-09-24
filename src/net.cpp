#include "net.h"

#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "config.h"
#include "log.h"
#include "owl/protocol.h"
#include "owl/wifi_test.h"

namespace owl::net {

static WifiPolicy policy(config::WIFI_CONNECT_MS, config::WIFI_WINDOW_MS);
static String ssid, pass;
static bool mdnsStarted = false;
static bool wasOnline = false;

enum class Job { None, Scan, Test };
static Job job = Job::None;
static bool scanRadioTemp = false;  // radio switched on just for a scan
static String testSsid, testPass;
static uint32_t testSince;
static volatile int lastReason = 0;  // last STA disconnect reason (WiFi event task)
static TestGate gate;
static void (*sink)(const char*) = nullptr;

static void emit(const char* json) {
    if (sink && json) sink(json);
}

static void loadCredentials() {
    Preferences p;
    if (p.begin("wifi", true)) {
        ssid = p.getString("ssid", "");
        pass = p.getString("pass", "");
        p.end();
    }
    if (ssid.length()) return;
    // One-time migration from firmware v1 (WiFiManager kept them in the ESP WiFi config).
    wifi_config_t c = {};
    WiFi.mode(WIFI_STA);
    if (esp_wifi_get_config(WIFI_IF_STA, &c) == ESP_OK && c.sta.ssid[0]) {
        saveCredentials(reinterpret_cast<const char*>(c.sta.ssid), reinterpret_cast<const char*>(c.sta.password));
        log::printf("wifi: migrated credentials for %s", ssid.c_str());
    }
    WiFi.mode(WIFI_OFF);
}

static void radioOn() {
    WiFi.persistent(false);  // credentials live in our NVS, not the ESP WiFi config
    WiFi.setHostname(config::HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid.c_str(), pass.c_str());
    log::printf("wifi: connecting to %s", ssid.c_str());
}

static void radioOff() {
    if (mdnsStarted) {
        MDNS.end();
        mdnsStarted = false;
    }
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    log::printf("wifi: off (%s)", policy.status() == WifiPolicy::Status::Failed ? "connect failed" : "window closed");
}

void begin() {
    WiFi.onEvent([](WiFiEvent_t, WiFiEventInfo_t info) { lastReason = info.wifi_sta_disconnected.reason; },
                 ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    loadCredentials();
    if (policy.begin(millis(), ssid.length() > 0) == WifiPolicy::Action::Start) radioOn();
    else log::printf("wifi: not configured");
}

static void finishScan(int n) {
    int count = 0;
    char buf[160];
    for (int i = 0; i < n && count < 20; ++i) {
        String name = WiFi.SSID(i);
        if (!name.length()) continue;
        bool dup = false;  // networks come sorted by RSSI: keep the first (strongest) per SSID
        for (int k = 0; k < i && !dup; ++k) dup = WiFi.SSID(k) == name;
        if (dup) continue;
        JsonWriter w(buf, sizeof(buf));
        w.str("type", "wifi_net").str("ssid", name.c_str()).num("rssi", WiFi.RSSI(i))
            .boolean("secure", WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        emit(w.finish());
        ++count;
    }
    JsonWriter w(buf, sizeof(buf));
    w.str("type", "wifi_scan_done").num("count", count);
    if (n < 0) w.str("msg", "scan failed");
    emit(w.finish());
    WiFi.scanDelete();
    if (scanRadioTemp && policy.link() == WifiPolicy::Link::Off) WiFi.mode(WIFI_OFF);
    scanRadioTemp = false;
    job = Job::None;
}

static void finishTest(bool ok, const char* msg) {
    char buf[160];
    JsonWriter w(buf, sizeof(buf));
    w.str("type", "wifi_test").boolean("ok", ok);
    if (ok) w.num("rssi", WiFi.RSSI());
    else w.str("msg", msg);
    emit(w.finish());
    log::printf("wifi: test %s: %s", testSsid.c_str(), ok ? "ok" : msg);
    gate.record(testSsid.c_str(), testPass.c_str(), ok);
    testPass = "";
    WiFi.disconnect();
    if (policy.link() != WifiPolicy::Link::Off) radioOn();  // back to the saved network
    else WiFi.mode(WIFI_OFF);
    job = Job::None;
}

static void runJobs() {
    if (job == Job::Scan) {
        int n = WiFi.scanComplete();
        if (n != WIFI_SCAN_RUNNING) finishScan(n);
    } else if (job == Job::Test) {
        int r = lastReason;
        if (WiFi.status() == WL_CONNECTED) finishTest(true, nullptr);
        else if (r == 201 || r == 202 || r == 15 || r == 204) finishTest(false, wifiFailReason(r));
        else if (millis() - testSince >= config::WIFI_CONNECT_MS) finishTest(false, wifiFailReason(r == 8 ? 0 : r));
    }
}

bool startScan() {
    if (job != Job::None) return false;
    if (WiFi.getMode() == WIFI_OFF) {
        WiFi.mode(WIFI_STA);
        scanRadioTemp = true;
    }
    if (WiFi.scanNetworks(true) == WIFI_SCAN_FAILED) {
        job = Job::Scan;
        finishScan(-1);
        return true;
    }
    job = Job::Scan;
    return true;
}

bool startTest(const char* s, const char* p) {
    if (job != Job::None || !*s) return false;
    job = Job::Test;
    testSsid = s;
    testPass = p;
    testSince = millis();
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    lastReason = 0;
    WiFi.begin(s, p);
    log::printf("wifi: testing %s", s);
    return true;
}

bool saveTested(const char* s, const char* p) {
    if (!gate.allowSave(s, p)) return false;
    saveCredentials(s, p);
    gate.clear();
    log::printf("wifi: saved credentials for %s", s);
    if (policy.link() != WifiPolicy::Link::Off) radioOn();  // switch networks now if WiFi is up
    return true;
}

void setEventSink(void (*s)(const char*)) { sink = s; }

const char* statusName() {
    static const char* const STATUS[] = {"unconfigured", "connecting", "connected", "failed", "off"};
    return STATUS[int(policy.status())];
}

String ssidName() { return ssid; }

void loop() {
    runJobs();
    if (job == Job::Test) return;  // the test owns the radio; the policy resumes afterwards
    bool connected = WiFi.status() == WL_CONNECTED;
    switch (policy.update(millis(), connected)) {
        case WifiPolicy::Action::Start: radioOn(); break;
        case WifiPolicy::Action::Stop: radioOff(); break;
        case WifiPolicy::Action::None: break;
    }
    bool on = online();
    if (on && !wasOnline) {
        log::printf("wifi: online %s as %s.local", WiFi.localIP().toString().c_str(), config::HOSTNAME);
        if (!mdnsStarted && MDNS.begin(config::HOSTNAME)) {
            MDNS.addService("http", "tcp", 80);
            mdnsStarted = true;
        }
    } else if (!on && wasOnline && policy.link() != WifiPolicy::Link::Off) {
        log::printf("wifi: link lost, reconnecting");
    }
    wasOnline = on;
}

bool online() { return policy.link() == WifiPolicy::Link::Online; }
bool configured() { return ssid.length() > 0; }
bool windowOpen() { return policy.windowOpen(millis()); }
WifiPolicy::Status status() { return policy.status(); }
bool devmode() { return policy.devmode(); }

void setDevmode(bool on) {
    if (on != policy.devmode()) log::printf("wifi: debug mode %s", on ? "on" : "off");
    policy.setDevmode(on);
}

void setHold(bool on) { policy.setHold(on); }

void saveCredentials(const char* s, const char* p) {
    Preferences prefs;
    if (prefs.begin("wifi", false)) {
        prefs.putString("ssid", s);
        prefs.putString("pass", p);
        prefs.end();
    }
    ssid = s;
    pass = p;
    policy.setConfigured(true);
}

void forgetCredentials() {
    Preferences prefs;
    if (prefs.begin("wifi", false)) {
        prefs.clear();
        prefs.end();
    }
    ssid = pass = "";
    policy.setConfigured(false);
}

void appendDebug(String& j) {
    char buf[240];
    snprintf(buf, sizeof(buf),
             "\"wifi_status\":\"%s\",\"wifi_window\":%s,\"devmode\":%s,\"ssid\":\"%s\",\"rssi\":%d,\"ip\":\"%s\"",
             statusName(), windowOpen() ? "true" : "false", devmode() ? "true" : "false",
             ssid.c_str(), online() ? int(WiFi.RSSI()) : 0,
             online() ? WiFi.localIP().toString().c_str() : "");
    j += buf;
}

}  // namespace owl::net
