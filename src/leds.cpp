#include "leds.h"

#include "config.h"
#include "owl/gamma.h"

namespace owl::leds {

CRGB strip[LAYOUT.numLeds];
CRGB output[LAYOUT.numLeds];

static GammaLut lut(config::GAMMA);
static uint32_t corr = config::COLOR_CORRECTION;

void begin() {
    FastLED.addLeds<WS2812B, config::LED_PIN, GRB>(output, LAYOUT.numLeds);
    FastLED.setMaxPowerInVoltsAndMilliamps(config::LED_VOLTS, config::LED_MAX_MILLIAMPS);
    FastLED.setCorrection(CRGB(corr));
    FastLED.clear(true);
}

void show() {
    for (int i = 0; i < LAYOUT.numLeds; ++i)
        output[i] = CRGB(lut[strip[i].r], lut[strip[i].g], lut[strip[i].b]);
    FastLED.show();
}

void setColor(uint32_t correction, float gamma) {
    corr = correction & 0xFFFFFF;
    FastLED.setCorrection(CRGB(corr));
    lut.set(gamma);
}

uint32_t correction() { return corr; }
float gamma() { return lut.gamma(); }

}  // namespace owl::leds
