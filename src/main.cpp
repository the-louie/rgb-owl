#include <Arduino.h>

#include "app.h"
#include "ble.h"
#include "clock.h"
#include "crash.h"
#include "http.h"
#include "net.h"
#include "ota.h"
#include "reset.h"

void setup() {
    Serial.begin(115200);
    owl::crash::begin();
    owl::clock::begin();
    owl::app::begin();
    owl::reset::begin();
    owl::ble::begin();
    owl::net::begin();
    owl::ota::begin();
}

void loop() {
    owl::app::loop();
    owl::clock::loop();
    owl::reset::loop();
    owl::ble::loop();
    owl::net::loop();
    owl::http::loop();
    owl::ota::loop();
}
