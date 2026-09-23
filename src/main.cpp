#include <Arduino.h>

#include "layout.h"

void setup() {
    Serial.begin(115200);
    Serial.printf("owl: %u LEDs, grid %ux%u\n", owl::config::OWL_LAYOUT.numLeds,
                  owl::config::OWL_LAYOUT.width, owl::config::OWL_LAYOUT.height);
}

void loop() {
    delay(1000);
}
