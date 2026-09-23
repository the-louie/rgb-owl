#pragma once
// Effect interface. Effects draw a full frame into a strip-ordered buffer,
// addressed by grid XY through the layout (so two effects can be crossfaded).

#include <FastLED.h>

#include "leds.h"
#include "owl/registry.h"

namespace owl {

struct Canvas {
    CRGB* px;  // LAYOUT.numLeds entries, strip order

    void set(int x, int y, const CRGB& c) {
        int16_t i = leds::LAYOUT.index(x, y);
        if (i != NO_LED) px[i] = c;
    }
    void fill(const CRGB& c) { fill_solid(px, leds::LAYOUT.numLeds, c); }
};

struct Frame {
    uint32_t t;   // effect time in ms (speed-scaled)
    uint32_t dt;  // effect time since previous frame
};

class Effect {
public:
    virtual ~Effect() = default;
    virtual const char* name() const = 0;
    virtual void start() {}  // called when the effect becomes active
    virtual void render(Canvas& c, const Frame& f) = 0;
};

// All effects, in auto-cycle order (defined in effects.cpp).
extern const Registry<Effect> EFFECTS;

}  // namespace owl
