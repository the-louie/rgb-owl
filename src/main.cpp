#include <Arduino.h>

#include "app.h"
#include "http.h"
#include "net.h"

void setup() {
    Serial.begin(115200);
    owl::app::begin();
    owl::net::begin();
}

void loop() {
    owl::app::loop();
    owl::net::loop();
    owl::http::loop();
}
