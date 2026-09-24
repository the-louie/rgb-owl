#include "http.h"

#include <WebServer.h>

#include <vector>

#include "app.h"
#include "ble.h"
#include "effect.h"
#include "leds.h"
#include "log.h"
#include "net.h"
#include "signature.h"

#include <esp_core_dump.h>

namespace owl::http {

extern const char INDEX_HTML[] asm("_binary_web_index_html_start");

static WebServer web(80);

static bool running = false;
static bool routed = false;

static void sendState(int code = 200) {
    char buf[200];
    size_t n = toJson(buf, sizeof(buf), app::settings(), app::currentEffect());
    web.send(code, "application/json", n ? buf : "{}");
}

static void sendError(String msg) {
    msg.replace("\\", "");  // msg echoes client input: keep the JSON well-formed
    msg.replace("\"", "");
    web.send(400, "application/json", "{\"error\":\"" + msg + "\"}");
}

// POST /api/state: form parameters as in Settings; effect may be an index or a name.
static void postState() {
    for (int i = 0; i < web.args(); ++i) {
        String key = web.argName(i);
        String value = web.arg(i);
        if (key == "plain") continue;  // raw body, not a form field
        if (key == "effect") {
            int idx = EFFECTS.find(value.c_str());
            if (idx >= 0) value = String(idx);
        }
        ApplyResult r = app::set(key.c_str(), value.c_str());
        if (r == ApplyResult::UnknownKey) return sendError("unknown setting: " + key);
        if (r == ApplyResult::BadValue) return sendError("bad value for " + key);
    }
    sendState();
}

static void getEffects() {
    String out = "[";
    for (size_t i = 0; i < EFFECTS.size(); ++i) {
        if (i) out += ',';
        out += '"';
        out += EFFECTS[i].name();
        out += '"';
    }
    out += ']';
    web.send(200, "application/json", out);
}

static void getDebug() {
    String j = "{";
    app::appendDebug(j);
    j += ',';
    net::appendDebug(j);
    j += '}';
    web.send(200, "application/json", j);
}

// POST /api/test: mode=none|off|solid|pixel|column|row|walk [&r&g&b] [&index]
static void postTest() {
    auto num = [](const char* k) { return web.hasArg(k) ? web.arg(k).toInt() : 0; };
    if (!app::setTest(web.arg("mode").c_str(), uint8_t(num("r")), uint8_t(num("g")), uint8_t(num("b")),
                      int(num("index"))))
        return sendError("bad test mode or index");
    getDebug();
}

// POST /api/devmode: password=<OTA password>&on=1|0 (reachable in the boot window).
static void postDevmode() {
    if (web.arg("password") != OWL_OTA_PASSWORD) {
        log::printf("http: devmode rejected, wrong password");
        return web.send(401, "application/json", "{\"error\":\"wrong password\"}");
    }
    net::setDevmode(web.arg("on") != "0");
    getDebug();
}

static void addRoutes() {
    web.on("/", HTTP_GET, devOnly([] { web.send(200, "text/html", INDEX_HTML); }));
    web.on("/api/state", HTTP_GET, devOnly([] { sendState(); }));
    web.on("/api/state", HTTP_POST, devOnly(postState));
    web.on("/api/effects", HTTP_GET, devOnly(getEffects));
    web.on("/api/debug", HTTP_GET, getDebug);  // also in the boot window
    web.on("/api/devmode", HTTP_POST, postDevmode);  // also in the boot window
    web.on("/api/frame", HTTP_GET, devOnly([] {
        // last rendered frame, strip order, before brightness/power scaling
        String out;
        out.reserve(leds::LAYOUT.numLeds * 7);
        char px[8];
        for (int i = 0; i < leds::LAYOUT.numLeds; ++i) {
            snprintf(px, sizeof(px), "%02x%02x%02x ", leds::strip[i].r, leds::strip[i].g, leds::strip[i].b);
            out += px;
        }
        web.send(200, "text/plain", out);
    }));
    web.on("/api/log", HTTP_GET, devOnly([] { web.send(200, "text/plain", log::dump()); }));
    web.on("/api/test", HTTP_POST, devOnly(postTest));
    // last panic from the core dump partition: task, PC, backtrace (decode with addr2line)
    web.on("/api/coredump", HTTP_GET, devOnly([] {
        esp_core_dump_summary_t s;
        if (esp_core_dump_get_summary(&s) != ESP_OK) return web.send(404, "application/json", "{\"error\":\"no core dump\"}");
        String j = "{\"task\":\"" + String(s.exc_task) + "\",\"pc\":\"0x" + String(s.exc_pc, HEX) + "\",\"backtrace\":[";
        for (uint32_t i = 0; i < s.exc_bt_info.depth; ++i) j += (i ? ",\"0x" : "\"0x") + String(s.exc_bt_info.bt[i], HEX) + "\"";
        j += "],\"corrupted\":" + String(s.exc_bt_info.corrupted ? "true" : "false") + "}";
        web.send(200, "application/json", j);
    }));
    // signature self-test: data=<hex>&sig=<hex DER> -> {"valid":bool}
    web.on("/api/verify", HTTP_POST, devOnly([] {
        auto unhex = [](const String& h, std::vector<uint8_t>& out) {
            for (size_t i = 0; i + 1 < h.length(); i += 2) out.push_back(uint8_t(strtoul(h.substring(i, i + 2).c_str(), nullptr, 16)));
        };
        std::vector<uint8_t> data, sig;
        unhex(web.arg("data"), data);
        unhex(web.arg("sig"), sig);
        signature::Verifier v;
        v.update(data.data(), data.size());
        web.send(200, "application/json", v.finish(sig.data(), sig.size()) ? "{\"valid\":true}" : "{\"valid\":false}");
    }));
    // runs a BLE command line (form field "line"); replies appear as "event:" lines in /api/log
    web.on("/api/cmd", HTTP_POST, devOnly([] {
        ble::execute(web.arg("line").c_str());
        web.send(200, "application/json", "{\"ok\":true}");
    }));
    // colour tuning: correction=RRGGBB (hex) & gamma=1.0..3.0; not persisted
    web.on("/api/color", HTTP_POST, devOnly([] {
        uint32_t corr = web.hasArg("correction") ? strtoul(web.arg("correction").c_str(), nullptr, 16)
                                                 : leds::correction();
        float g = web.hasArg("gamma") ? web.arg("gamma").toFloat() : leds::gamma();
        if (g < 1.0f || g > 3.0f) return sendError("gamma must be 1.0-3.0");
        leds::setColor(corr, g);
        log::printf("color: correction %06lx gamma %.2f", (unsigned long)corr, double(g));
        getDebug();
    }));
    web.on("/api/reboot", HTTP_POST, devOnly([] {
        log::printf("reboot requested over HTTP");
        web.send(200, "application/json", "{\"ok\":true}");
        delay(300);
        ESP.restart();
    }));
    web.onNotFound([] { web.send(404, "text/plain", "not found"); });
}

void loop() {
    bool want = net::online();
    if (want != running) {
        if (want) {
            if (!routed) addRoutes(), routed = true;
            web.begin();
        } else {
            web.stop();
        }
        running = want;
    }
    if (running) web.handleClient();
}

WebServer& server() { return web; }

std::function<void()> devOnly(std::function<void()> handler) {
    return [handler] {
        if (net::devmode()) return handler();
        web.send(403, "application/json", "{\"error\":\"debug mode is off\"}");
    };
}

}  // namespace owl::http
