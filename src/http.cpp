#include "http.h"

#include <WebServer.h>

#include "app.h"
#include "effect.h"
#include "log.h"
#include "net.h"

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

static void addRoutes() {
    web.on("/", HTTP_GET, [] { web.send(200, "text/html", INDEX_HTML); });
    web.on("/api/state", HTTP_GET, [] { sendState(); });
    web.on("/api/state", HTTP_POST, postState);
    web.on("/api/effects", HTTP_GET, getEffects);
    web.on("/api/debug", HTTP_GET, getDebug);
    web.on("/api/log", HTTP_GET, [] { web.send(200, "text/plain", log::dump()); });
    web.on("/api/test", HTTP_POST, postTest);
    web.on("/api/reboot", HTTP_POST, [] {
        log::printf("reboot requested over HTTP");
        web.send(200, "application/json", "{\"ok\":true}");
        delay(300);
        ESP.restart();
    });
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

}  // namespace owl::http
