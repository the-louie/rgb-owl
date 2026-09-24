#include "ota.h"

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <FastLED.h>
#include <Update.h>
#include <WebServer.h>

#include "config.h"
#include "log.h"
#include "http.h"
#include "net.h"
#include "rollback.h"

namespace owl::ota {

static bool started = false;
static bool rejected = false;  // current web upload has a wrong password

static void blankLeds() { FastLED.clear(true); }  // the render loop is stalled while flashing

static void uploadDone() {
    WebServer& web = http::server();
    if (rejected) {
        web.send(401, "application/json", "{\"error\":\"wrong password\"}");
        return;
    }
    bool ok = !Update.hasError();
    web.send(ok ? 200 : 500, "application/json",
             ok ? "{\"ok\":true}" : "{\"error\":\"update failed\"}");
    if (ok) {
        delay(300);
        ESP.restart();
    }
}

static void uploadChunk() {
    HTTPUpload& u = http::server().upload();
    switch (u.status) {
        case UPLOAD_FILE_START:
            // the password field precedes the file in the multipart body
            rejected = http::server().arg("password") != OWL_OTA_PASSWORD;
            if (rejected) {
                log::printf("ota: web upload rejected, wrong password");
                break;
            }
            log::printf("ota: web upload %s", u.filename.c_str());
            blankLeds();
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
            break;
        case UPLOAD_FILE_WRITE:
            if (!rejected && Update.write(u.buf, u.currentSize) != u.currentSize) Update.printError(Serial);
            break;
        case UPLOAD_FILE_END:
            if (!rejected && !Update.end(true)) Update.printError(Serial);
            else if (!rejected) rollback::expectNewImage();
            break;
        case UPLOAD_FILE_ABORTED:
            if (!rejected) Update.abort();
            break;
    }
}

void begin() {
    http::server().on("/update", HTTP_POST, http::devOnly(uploadDone), http::devOnly(uploadChunk));
    ArduinoOTA.setHostname(config::HOSTNAME);
    ArduinoOTA.setPassword(OWL_OTA_PASSWORD);
    ArduinoOTA.setMdnsEnabled(false);  // net.cpp owns mDNS
    ArduinoOTA.onStart([] {
        log::printf("ota: ArduinoOTA update started");
        blankLeds();
    });
    ArduinoOTA.onEnd([] { rollback::expectNewImage(); });
    ArduinoOTA.onError([](ota_error_t e) { log::printf("ota: error %u", unsigned(e)); });
}

void loop() {
    bool want = net::online() && net::devmode();  // ArduinoOTA only in debug mode
    if (want && !started) {
        ArduinoOTA.begin();
        MDNS.enableArduino(3232);
        started = true;
    } else if (!want && started) {
        ArduinoOTA.end();
        started = false;
    }
    if (started) ArduinoOTA.handle();
}

}  // namespace owl::ota
