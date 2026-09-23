#include <Arduino.h>

#include "config.h"
#include "effect.h"
#include "leds.h"
#include "owl/cycle.h"
#include "owl/timing.h"

using namespace owl;

static FramePacer pacer(1000 / config::FPS);
static uint32_t bootMs;
static uint32_t lastMs;
static EffectClock effectClock;
static Cycler cycler(EFFECTS.size(), config::CYCLE_INTERVAL_MS, config::CYCLE_FADE_MS);
static CRGB fadeBuf[leds::LAYOUT.numLeds];
static Canvas canvas{leds::strip};
static Canvas fadeCanvas{fadeBuf};
static bool walking = true;

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

static void renderEffects(uint32_t dt) {
    uint32_t before = effectClock.now();
    uint32_t t = effectClock.advance(dt, 128);
    Frame frame{t, t - before};

    Cycler::State s = cycler.update(dt);
    if (s.started >= 0) {
        EFFECTS[size_t(s.started)].start();
        Serial.printf("effect: %s\n", EFFECTS[size_t(s.started)].name());
    }
    EFFECTS[s.current].render(canvas, frame);
    if (s.next >= 0) {
        EFFECTS[size_t(s.next)].render(fadeCanvas, frame);
        nblend(leds::strip, fadeBuf, leds::LAYOUT.numLeds, s.mix);
    }
}

void setup() {
    Serial.begin(115200);
    leds::begin();
    random16_set_seed(uint16_t(esp_random()));
    bootMs = lastMs = millis();
    Serial.printf("owl: %u LEDs, grid %ux%u, %u effects\n", leds::LAYOUT.numLeds,
                  leds::LAYOUT.width, leds::LAYOUT.height, unsigned(EFFECTS.size()));
}

void loop() {
    uint32_t now = millis();
    if (!pacer.due(now)) return;
    uint32_t dt = now - lastMs;
    lastMs = now;
    if (walking && !renderWalk(now - bootMs)) {
        walking = false;
        EFFECTS[cycler.current()].start();
    }
    if (!walking) renderEffects(dt);
    leds::show();
}
