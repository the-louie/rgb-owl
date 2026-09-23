#include <Arduino.h>

#include "app.h"

void setup() {
    Serial.begin(115200);
    owl::app::begin();
}

void loop() {
    owl::app::loop();
}
