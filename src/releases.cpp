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
        JsonDocument doc;
        DeserializationError e = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
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
