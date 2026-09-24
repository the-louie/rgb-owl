#include "releases.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "owl/release.h"

extern const uint8_t CA_BUNDLE[] asm("_binary_data_x509_crt_bundle_bin_start");

namespace owl::releases {

constexpr const char* IMAGE_ASSET = "owl-firmware.bin";
constexpr const char* SIG_ASSET = "owl-firmware.bin.sig";
constexpr size_t MAX_RELEASES = 10;
constexpr size_t MAX_BODY = 512 * 1024;  // PSRAM; 10 releases with notes are typically < 100 KB
constexpr uint32_t STALL_MS = 15000;

static volatile State st = State::Idle;
static Result res;
static String repo;
static bool pre;

static void run(void*) {
    Result r;
    WiFiClientSecure tls;
    tls.setCACertBundle(CA_BUNDLE);
    HTTPClient http;
    http.useHTTP10(true);  // no chunked encoding, so the JSON can be parsed straight off the stream
    http.setUserAgent("owl-firmware");
    http.setTimeout(15000);
    String url = "https://api.github.com/repos/" + repo + "/releases?per_page=" + String(MAX_RELEASES);
    int code = http.begin(tls, url) ? http.GET() : -1;
    if (code != HTTP_CODE_OK) {
        r.error = code == 404 ? "project not found" : "GitHub request failed (" + String(code) + ")";
    } else {
        JsonDocument filter;  // keep only what we need from the (large) response
        filter[0]["tag_name"] = true;
        filter[0]["prerelease"] = true;
        filter[0]["draft"] = true;
        filter[0]["assets"][0]["name"] = true;
        filter[0]["assets"][0]["browser_download_url"] = true;
        // Read the body ourselves: deserializeJson(stream) waits for data in Stream::timedRead(),
        // which busy-polls and starved IDLE0 into a task-watchdog reset (T-44, seen on the owl).
        char* body = static_cast<char*>(ps_malloc(MAX_BODY));
        size_t len = 0;
        WiFiClient* s = http.getStreamPtr();
        uint32_t last = millis();
        while (body && len < MAX_BODY && (s->connected() || s->available())) {
            int n = s->available();
            if (n <= 0) {
                if (millis() - last > STALL_MS) break;
                delay(10);  // let IDLE0 run
                continue;
            }
            len += s->read(reinterpret_cast<uint8_t*>(body) + len, min(size_t(n), MAX_BODY - len));
            last = millis();
        }
        JsonDocument doc;
        DeserializationError e = body ? deserializeJson(doc, body, len, DeserializationOption::Filter(filter))
                                      : DeserializationError::NoMemory;
        free(body);
        if (e) {
            r.error = String("bad GitHub response: ") + e.c_str();
        } else {
            JsonArray list = doc.as<JsonArray>();
            ReleaseEntry entries[MAX_RELEASES];
            size_t n = 0;
            for (JsonObject o : list) {
                if (n == MAX_RELEASES) break;
                entries[n++] = {o["tag_name"] | "", o["prerelease"] | false, o["draft"] | false};
            }
            int best = pickRelease(entries, n, pre);
            if (best >= 0) {
                JsonObject o = list[best];
                r.tag = o["tag_name"].as<const char*>();
                for (JsonObject a : o["assets"].as<JsonArray>()) {
                    if (a["name"] == IMAGE_ASSET) r.imageUrl = a["browser_download_url"].as<const char*>();
                    if (a["name"] == SIG_ASSET) r.sigUrl = a["browser_download_url"].as<const char*>();
                }
                r.newer = isNewer(r.tag.c_str(), OWL_VERSION);
                if (r.newer && (r.imageUrl.isEmpty() || r.sigUrl.isEmpty()))
                    r.error = "release " + r.tag + " has no signed firmware";
            }
        }
    }
    http.end();
    res = r;
    st = r.error.isEmpty() ? State::Done : State::Failed;
    vTaskDelete(nullptr);
}

bool start(const String& project, bool includePre) {
    if (st == State::Checking) return false;
    repo = project;
    pre = includePre;
    st = State::Checking;
    if (xTaskCreatePinnedToCore(run, "releases", 12288, nullptr, 1, nullptr, 0) != pdPASS) {
        res = Result{};
        res.error = "no memory for release check";
        st = State::Failed;
        return false;
    }
    return true;
}

State state() { return st; }
const Result& result() { return res; }
void reset() {
    if (st != State::Checking) st = State::Idle;
}

}  // namespace owl::releases
