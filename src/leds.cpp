#include "leds.h"

#include "config.h"

namespace owl::leds {

CRGB strip[LAYOUT.numLeds];

void begin() {
    FastLED.addLeds<WS2812B, config::LED_PIN, GRB>(strip, LAYOUT.numLeds);
    FastLED.setMaxPowerInVoltsAndMilliamps(config::LED_VOLTS, config::LED_MAX_MILLIAMPS);
    FastLED.clear(true);
}

void show() { FastLED.show(); }

}  // namespace owl::leds
