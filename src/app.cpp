#include "app.h"

#include <Arduino.h>

#include "config.h"
#include "effect.h"
#include "effects/effects.h"
#include "leds.h"
#include "owl/cycle.h"
#include "owl/debounce.h"
#include "owl/timing.h"
#include "settings_store.h"

namespace owl::app {

static Settings cfg;
static SaveDebouncer saver(config::SAVE_DELAY_MS);
static FramePacer pacer(1000 / config::FPS);
static uint32_t bootMs;
static uint32_t lastMs;
static EffectClock effectClock;
static Cycler cycler(EFFECTS.size(), cfg.intervalS * 1000UL, cfg.fadeMs);
static CRGB fadeBuf[leds::LAYOUT.numLeds];
static Canvas canvas{leds::strip};
static Canvas fadeCanvas{fadeBuf};
static bool walking = true;

// Pushes settings to the hardware and scheduler (idempotent).
static void applyAll() {
    FastLED.setBrightness(cfg.brightness);
    cycler.setAuto(cfg.autoCycle);
    cycler.setInterval(cfg.intervalS * 1000UL);
    cycler.setFade(cfg.fadeMs);
    effects::setBreathingHue(cfg.hue);
}

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
    uint32_t t = effectClock.advance(dt, cfg.speed);
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

void begin() {
    store::load(cfg);
    if (cfg.effect >= EFFECTS.size()) cfg.effect = 0;
    leds::begin();
    applyAll();
    cycler.select(cfg.effect);
    cycler.update(cfg.fadeMs);  // start on the saved effect without a fade
    random16_set_seed(uint16_t(esp_random()));
    bootMs = lastMs = millis();
    Serial.printf("owl: %u LEDs, grid %ux%u, %u effects\n", leds::LAYOUT.numLeds,
                  leds::LAYOUT.width, leds::LAYOUT.height, unsigned(EFFECTS.size()));
}

void loop() {
    uint32_t now = millis();
    if (saver.due(now)) store::save(cfg);
    if (!pacer.due(now)) return;
    uint32_t dt = now - lastMs;
    lastMs = now;
    if (walking && !renderWalk(now - bootMs)) {
        walking = false;
        EFFECTS[cycler.current()].start();
    }
    if (!walking) {
        if (cfg.on) {
            renderEffects(dt);
        } else {
            fill_solid(leds::strip, leds::LAYOUT.numLeds, CRGB::Black);
        }
    }
    leds::show();
}

const Settings& settings() { return cfg; }

ApplyResult set(const char* key, const char* value) {
    Settings next = cfg;
    ApplyResult r = apply(next, key, value, EFFECTS.size());
    if (r != ApplyResult::Ok || next == cfg) return r;
    bool effectChanged = next.effect != cfg.effect;
    cfg = next;
    applyAll();
    if (effectChanged) cycler.select(cfg.effect);
    saver.changed(millis());
    return r;
}

size_t currentEffect() { return cycler.current(); }

}  // namespace owl::app
