#include "ota.h"

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <FastLED.h>
#include <Update.h>
#include <WebServer.h>

#include "config.h"
#include "http.h"
#include "net.h"

namespace owl::ota {

static bool started = false;

static void blankLeds() { FastLED.clear(true); }  // the render loop is stalled while flashing

static void uploadDone() {
    WebServer& web = http::server();
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
            Serial.printf("ota: web upload %s\n", u.filename.c_str());
            blankLeds();
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
            break;
        case UPLOAD_FILE_WRITE:
            if (Update.write(u.buf, u.currentSize) != u.currentSize) Update.printError(Serial);
            break;
        case UPLOAD_FILE_END:
            if (!Update.end(true)) Update.printError(Serial);
            break;
        case UPLOAD_FILE_ABORTED:
            Update.abort();
            break;
    }
}

void begin() {
    http::server().on("/update", HTTP_POST, uploadDone, uploadChunk);
    ArduinoOTA.setHostname(config::HOSTNAME);
    ArduinoOTA.setMdnsEnabled(false);  // net.cpp owns mDNS
    ArduinoOTA.onStart(blankLeds);
    ArduinoOTA.onError([](ota_error_t e) { Serial.printf("ota: error %u\n", unsigned(e)); });
}

void loop() {
    if (!started && net::online()) {
        ArduinoOTA.begin();
        MDNS.enableArduino(3232);
        started = true;
    }
    if (started) ArduinoOTA.handle();
}

}  // namespace owl::ota
