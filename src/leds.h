#pragma once
// LED output: FastLED on the strip defined by include/layout.h.

#include <FastLED.h>

#include "layout.h"

namespace owl::leds {

inline constexpr auto& LAYOUT = config::OWL_LAYOUT;
extern CRGB strip[LAYOUT.numLeds];

void begin();
void show();

// Colour of the strip LED at grid (x, y); cells without a LED are ignored.
inline void setXY(int x, int y, const CRGB& c) {
    int16_t i = LAYOUT.index(x, y);
    if (i != NO_LED) strip[i] = c;
}

}  // namespace owl::leds
