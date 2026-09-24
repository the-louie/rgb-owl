#include "reset.h"

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <WiFi.h>

#include "config.h"
#include "leds.h"
#include "log.h"
#include "owl/hold.h"

namespace owl::reset {

static HoldDetector hold(config::RESET_HOLD_MS);

void begin() { pinMode(config::BOOT_BUTTON_PIN, INPUT_PULLUP); }  // 10k pull-up on the board too

void loop() {
    if (!hold.update(digitalRead(config::BOOT_BUTTON_PIN) == LOW, millis())) return;
    log::printf("reset: BOOT held %lu ms, forgetting bonds + WiFi", (unsigned long)config::RESET_HOLD_MS);
    NimBLEDevice::deleteAllBonds();
    WiFi.disconnect(true, true);  // erase stored credentials
    for (int i = 0; i < 3; ++i) {  // visible confirmation: three red flashes
        fill_solid(leds::strip, leds::LAYOUT.numLeds, CRGB::Red);
        leds::show();
        delay(200);
        fill_solid(leds::strip, leds::LAYOUT.numLeds, CRGB::Black);
        leds::show();
        delay(200);
    }
    ESP.restart();
}

}  // namespace owl::reset
