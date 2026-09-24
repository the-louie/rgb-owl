#pragma once
// LED output: FastLED on the strip defined by include/layout.h.

#include <FastLED.h>

#include "layout.h"

namespace owl::leds {

inline constexpr auto& LAYOUT = config::OWL_LAYOUT;
extern CRGB strip[LAYOUT.numLeds];  // what effects draw (perceptual values)
extern CRGB output[LAYOUT.numLeds];  // what is sent: strip through the gamma LUT

void begin();
void show();  // strip -> gamma -> output -> FastLED (colour correction, brightness, power cap)

// Colour tuning (debug; not persisted — bake the result into include/config.h).
void setColor(uint32_t correction, float gamma);
uint32_t correction();
float gamma();

// Colour of the strip LED at grid (x, y); cells without a LED are ignored.
inline void setXY(int x, int y, const CRGB& c) {
    int16_t i = LAYOUT.index(x, y);
    if (i != NO_LED) strip[i] = c;
}

}  // namespace owl::leds
