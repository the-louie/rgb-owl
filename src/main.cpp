#include <Arduino.h>

#include "app.h"
#include "http.h"
#include "net.h"
#include "ota.h"

void setup() {
    Serial.begin(115200);
    owl::app::begin();
    owl::net::begin();
    owl::ota::begin();
}

void loop() {
    owl::app::loop();
    owl::net::loop();
    owl::http::loop();
    owl::ota::loop();
}
