#include <Arduino.h>

#include "config.h"
#include "effect.h"
#include "leds.h"
#include "owl/timing.h"

using namespace owl;

static FramePacer pacer(1000 / config::FPS);
static uint32_t bootMs;
static uint32_t lastMs;
static EffectClock effectClock;
static size_t current = 0;
static Canvas canvas{leds::strip};

// Boot test pattern: walks the strip in wiring order, one hue per column,
// so the layout in include/layout.h can be checked against the hardware.
static bool renderWalk(uint32_t elapsedMs) {
    int32_t i = walkIndex(elapsedMs, config::WALK_STEP_MS, leds::LAYOUT.numLeds);
    fill_solid(leds::strip, leds::LAYOUT.numLeds, CRGB::Black);
    if (i < 0) return false;
    uint8_t x = leds::LAYOUT.pos[i].x;
    leds::strip[i] = CHSV(uint8_t(x * 256 / leds::LAYOUT.width), 200, 255);
    return true;
}

void setup() {
    Serial.begin(115200);
    leds::begin();
    bootMs = lastMs = millis();
    EFFECTS[current].start();
    Serial.printf("owl: %u LEDs, grid %ux%u\n", leds::LAYOUT.numLeds, leds::LAYOUT.width,
                  leds::LAYOUT.height);
}

void loop() {
    uint32_t now = millis();
    if (!pacer.due(now)) return;
    uint32_t dt = now - lastMs;
    lastMs = now;
    if (!renderWalk(now - bootMs)) {
        uint32_t before = effectClock.now();
        uint32_t t = effectClock.advance(dt, 128);
        EFFECTS[current].render(canvas, Frame{t, t - before});
    }
    leds::show();
}
