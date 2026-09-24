#include <Arduino.h>

#include "app.h"
#include "ble.h"
#include "clock.h"
#include "crash.h"
#include "http.h"
#include "net.h"
#include "ota.h"
#include "reset.h"
#include "rollback.h"
#include "update.h"

void setup() {
    Serial.begin(115200);
    owl::crash::begin();
    owl::rollback::begin();
    owl::clock::begin();
    owl::update::begin();
    owl::app::begin();
    owl::reset::begin();
    owl::ble::begin();
    owl::net::begin();
    owl::ota::begin();
}

void loop() {
#ifdef OWL_CRASH_TEST  // rollback test image: panics 10 s after boot
    if (millis() > 10000) abort();
#endif
    owl::app::loop();
    owl::clock::loop();
    owl::reset::loop();
    owl::update::loop();
    owl::rollback::loop();
    owl::ble::loop();
    owl::net::loop();
    owl::http::loop();
    owl::ota::loop();
}
