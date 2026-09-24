#include "installer.h"

#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>

#include "signature.h"

extern const uint8_t CA_BUNDLE[] asm("_binary_data_x509_crt_bundle_bin_start");

namespace owl::installer {

constexpr uint32_t STALL_MS = 20000;  // give up if no data arrives for this long

static volatile State st = State::Idle;
static volatile uint8_t pct = 0;
static const char* err = "";
static String image, sig;

static void fail(const char* e) {
    err = e;
    st = State::Failed;
}

// Opens url with redirects (GitHub release assets redirect to a CDN).
static bool open(HTTPClient& http, WiFiClientSecure& tls, WiFiClient& plain, const String& url) {
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(15000);
    http.setUserAgent("owl-firmware");
    bool ok = url.startsWith("https://") ? http.begin(tls, url) : http.begin(plain, url);
    return ok && http.GET() == HTTP_CODE_OK;
}

static void run(void*) {
    WiFiClientSecure tls;
    tls.setCACertBundle(CA_BUNDLE);
    WiFiClient plain;
    uint8_t sigBuf[128];
    size_t sigLen = 0;
    {
        HTTPClient http;
        if (!open(http, tls, plain, sig)) {
            fail("signature download failed");
        } else {
            String body = http.getString();
            sigLen = body.length();
            if (sigLen == 0 || sigLen > sizeof(sigBuf)) fail("bad signature file");
            else memcpy(sigBuf, body.c_str(), sigLen);
        }
        http.end();
    }
    if (st != State::Failed) {
        HTTPClient http;
        int total = 0;
        if (!open(http, tls, plain, image)) fail("image download failed");
        else if ((total = http.getSize()) <= 0) fail("image size unknown");
        else if (!Update.begin(size_t(total))) fail("image does not fit the OTA slot");
        if (st != State::Failed) {
            signature::Verifier verifier;
            WiFiClient* stream = http.getStreamPtr();
            static uint8_t buf[4096];
            int done = 0;
            uint32_t last = millis();
            while (done < total && st != State::Failed) {
                size_t avail = stream->available();
                if (!avail) {
                    if (!http.connected() || millis() - last > STALL_MS) fail("download stalled");
                    delay(5);
                    continue;
                }
                int n = stream->readBytes(buf, min(avail, sizeof(buf)));
                verifier.update(buf, n);
                if (Update.write(buf, n) != size_t(n)) fail("flash write failed");
                done += n;
                last = millis();
                pct = uint8_t(uint64_t(done) * 100 / total);
            }
            if (st != State::Failed) {
                st = State::Verifying;
                if (!verifier.finish(sigBuf, sigLen)) fail("bad signature");
                else if (!Update.end()) fail("image check failed");
                else st = State::Done;  // new slot set as boot partition; caller restarts
            }
            if (st == State::Failed) Update.abort();
        }
        http.end();
    }
    vTaskDelete(nullptr);
}

bool start(const String& imageUrl, const String& sigUrl) {
    if (st == State::Downloading || st == State::Verifying || st == State::Done) return false;
    image = imageUrl;
    sig = sigUrl;
    pct = 0;
    err = "";
    st = State::Downloading;
    // core 0 (WiFi/BLE); the LED loop keeps running on core 1
    if (xTaskCreatePinnedToCore(run, "installer", 12288, nullptr, 1, nullptr, 0) != pdPASS) {
        fail("no memory for installer task");
        return false;
    }
    return true;
}

State state() { return st; }
uint8_t percent() { return pct; }
const char* error() { return err; }
void reset() {
    if (st == State::Failed) st = State::Idle;
}

}  // namespace owl::installer
