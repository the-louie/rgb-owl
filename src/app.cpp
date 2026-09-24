#include "app.h"

#include <Arduino.h>

#include "config.h"
#include "log.h"
#include "effect.h"
#include "effects/effects.h"
#include "leds.h"
#include "ble.h"
#include "net.h"
#include "owl/boot_status.h"
#include "owl/cycle.h"
#include "owl/debounce.h"
#include "owl/stats.h"
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
static bool booting = true;  // boot status phases are shown until done
static bool statusStarted = false;
static BootStatus bootStatus;
static FrameStats stats;

enum class Test { None, Off, Solid, Pixel, Column, Row, Walk };
static const char* const TEST_NAMES[] = {"none", "off", "solid", "pixel", "column", "row", "walk"};
static Test test = Test::None;
static CRGB testColor;
static int testIndex = 0;
static uint32_t testStartMs = 0;

// Pushes settings to the hardware and scheduler (idempotent).
static void applyAll() {
    FastLED.setBrightness(cfg.brightness);
    cycler.setAuto(cfg.autoCycle);
    cycler.setMask(cfg.cycle);
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

static void renderTest(uint32_t now) {
    const auto& L = leds::LAYOUT;
    fill_solid(leds::strip, L.numLeds, CRGB::Black);
    switch (test) {
        case Test::Solid: fill_solid(leds::strip, L.numLeds, testColor); break;
        case Test::Pixel: leds::strip[testIndex] = testColor; break;
        case Test::Column:
            for (int y = 0; y < L.height; ++y) leds::setXY(L.width - 1 - testIndex, y, testColor);
            break;
        case Test::Row:
            for (int x = 0; x < L.width; ++x) leds::setXY(x, testIndex, testColor);
            break;
        case Test::Walk: {
            uint32_t cycle = uint32_t(config::WALK_STEP_MS) * L.numLeds;
            renderWalk((now - testStartMs) % cycle);
            break;
        }
        default: break;
    }
}

// Draws one boot status visual on the whole owl (SPEC v2 §Boot status).
static void renderStatus(const Visual& v, uint32_t now) {
    const auto& L = leds::LAYOUT;
    CRGB c(v.r, v.g, v.b);
    switch (v.kind) {
        case Visual::Solid: break;
        case Visual::Pulse: c.nscale8(uint8_t(40 + scale8(sin8(uint8_t(now * 256 / 1000)), 215))); break;
        case Visual::Blink: if ((now / 100) % 2) c = CRGB::Black; break;
        case Visual::Fill: {
            int lit = (L.height * v.fill + 99) / 100;  // rows lit from the bottom
            for (int i = 0; i < L.numLeds; ++i) {
                CRGB px = c;
                if (L.pos[i].y >= lit) px.nscale8(25);
                leds::strip[i] = px;
            }
            return;
        }
    }
    fill_solid(leds::strip, L.numLeds, c);
}

static void renderEffects(uint32_t dt) {
    uint32_t before = effectClock.now();
    uint32_t t = effectClock.advance(dt, cfg.speed);
    Frame frame{t, t - before};

    Cycler::State s = cycler.update(dt);
    if (s.started >= 0) {
        EFFECTS[size_t(s.started)].start();
        log::printf("effect: %s", EFFECTS[size_t(s.started)].name());
    }
    EFFECTS[s.current].render(canvas, frame);
    if (s.next >= 0) {
        EFFECTS[size_t(s.next)].render(fadeCanvas, frame);
        nblend(leds::strip, fadeBuf, leds::LAYOUT.numLeds, s.mix);
    }
}

static const char* resetReason() {
    switch (esp_reset_reason()) {
        case ESP_RST_POWERON: return "power-on";
        case ESP_RST_SW: return "software";
        case ESP_RST_PANIC: return "panic";
        case ESP_RST_INT_WDT: return "interrupt-watchdog";
        case ESP_RST_TASK_WDT: return "task-watchdog";
        case ESP_RST_WDT: return "watchdog";
        case ESP_RST_BROWNOUT: return "brownout";
        case ESP_RST_DEEPSLEEP: return "deep-sleep";
        case ESP_RST_EXT: return "external";
        default: return "other";
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
    log::printf("owl: boot, version %s, reset reason %s", OWL_VERSION, resetReason());
    log::printf("owl: %u LEDs, grid %ux%u, %u effects", leds::LAYOUT.numLeds,
                  leds::LAYOUT.width, leds::LAYOUT.height, unsigned(EFFECTS.size()));
}

void loop() {
    uint32_t now = millis();
    if (saver.due(now)) store::save(cfg);
    if (!pacer.due(now)) return;
    uint32_t dt = now - lastMs;
    lastMs = now;
    if (!statusStarted) {  // first loop: BLE and WiFi are initialised by now
        statusStarted = true;
        bootStatus.begin(now, ble::bondCount() > 0);
    }
    if (booting && test == Test::None) {
        renderStatus(bootStatus.update(now, net::status(), UpdateStatus::None, 0), now);
        if (bootStatus.done()) {
            booting = false;
            EFFECTS[cycler.current()].start();
            log::printf("owl: boot status done after %lu ms", (unsigned long)(now - bootMs));
        }
    } else {
        booting = false;
        if (test != Test::None) {
            renderTest(now);
        } else if (cfg.on) {
            renderEffects(dt);
        } else {
            fill_solid(leds::strip, leds::LAYOUT.numLeds, CRGB::Black);
        }
    }
    uint32_t t0 = micros();
    leds::show();
    stats.record(micros() - t0, now);
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

bool setTest(const char* mode, uint8_t r, uint8_t g, uint8_t b, int index) {
    const auto& L = leds::LAYOUT;
    int k = -1;
    for (int i = 0; i < int(sizeof(TEST_NAMES) / sizeof(TEST_NAMES[0])); ++i)
        if (!strcmp(mode, TEST_NAMES[i])) k = i;
    if (k < 0) return false;
    Test t = Test(k);
    int limit = t == Test::Pixel ? L.numLeds : t == Test::Column ? L.width : t == Test::Row ? L.height : 1;
    if (index < 0 || index >= limit) return false;
    test = t;
    testIndex = index;
    testColor = (r | g | b) ? CRGB(r, g, b) : CRGB(255, 255, 255);
    testStartMs = millis();
    if (t == Test::None) EFFECTS[cycler.current()].start();
    log::printf("test: %s index=%d rgb=%u,%u,%u", mode, index, testColor.r, testColor.g, testColor.b);
    return true;
}

void appendDebug(String& j) {
    const auto& L = leds::LAYOUT;
    // output holds the gamma-mapped frame; FastLED applies correction, brightness + power limit at show()
    uint32_t mW = calculate_unscaled_power_mW(leds::output, L.numLeds);
    uint8_t limited = calculate_max_brightness_for_power_mW(
        leds::output, L.numLeds, cfg.brightness, config::LED_VOLTS * config::LED_MAX_MILLIAMPS);
    char buf[640];
    snprintf(buf, sizeof(buf),
             "\"version\":\"%s\",\"build\":\"%s %s\",\"uptime_s\":%lu,\"reset_reason\":\"%s\","
             "\"heap_free\":%u,\"heap_min\":%u,\"psram_free\":%u,\"cpu_mhz\":%u,"
             "\"fps\":%lu,\"show_max_us\":%lu,\"leds\":%u,\"grid\":\"%ux%u\",\"data_pin\":%u,"
             "\"fastled\":%u,\"effect\":\"%s\",\"test\":\"%s\",\"boot_status\":%s,"
             "\"brightness\":%u,\"brightness_after_power_limit\":%u,\"est_ma\":%lu,"
             "\"power_limit_ma\":%lu,\"correction\":\"%06lx\",\"gamma\":%.2f",
             OWL_VERSION, __DATE__, __TIME__, (unsigned long)(millis() / 1000), resetReason(),
             unsigned(ESP.getFreeHeap()), unsigned(ESP.getMinFreeHeap()),
             unsigned(ESP.getFreePsram()), unsigned(ESP.getCpuFreqMHz()),
             (unsigned long)stats.fps(), (unsigned long)stats.maxFrameUs(), L.numLeds, L.width,
             L.height, config::LED_PIN, unsigned(FASTLED_VERSION), EFFECTS[cycler.current()].name(),
             TEST_NAMES[int(test)], booting ? "true" : "false", cfg.brightness, limited,
             (unsigned long)(uint64_t(mW) * limited / 255 / config::LED_VOLTS),
             (unsigned long)config::LED_MAX_MILLIAMPS, (unsigned long)leds::correction(),
             double(leds::gamma()));
    j += buf;
}

}  // namespace owl::app
