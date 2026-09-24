#pragma once
// User-adjustable settings (web UI / NVS). Hardware-independent; unit-tested on host.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace owl {

struct Settings {
    bool on = true;
    uint8_t effect = 0;          // index into EFFECTS
    bool autoCycle = true;
    uint16_t intervalS = 60;     // time per effect when auto-cycling
    uint16_t fadeMs = 2000;      // crossfade duration
    uint8_t brightness = 128;
    uint8_t speed = 128;         // 128 = 1x
    uint8_t hue = 160;           // breathing colour
    // Fields below are sent in the `config` event, not the (MTU-limited) state notification.
    // NVS stores this struct as a blob: only ever APPEND fields (see settings_store.cpp).
    uint32_t cycle = 0x7FFFFFFF;  // bit i = effect i in auto-cycle
    bool night = true;            // LEDs off between nightFrom and nightTo (local time)
    uint16_t nightFrom = 23 * 60; // minutes after midnight
    uint16_t nightTo = 7 * 60;

    static constexpr uint16_t INTERVAL_MIN_S = 5;
    static constexpr uint16_t INTERVAL_MAX_S = 3600;
    static constexpr uint16_t FADE_MAX_MS = 10000;
    static constexpr uint8_t BRIGHTNESS_MIN = 1;

    bool operator==(const Settings& o) const {
        return on == o.on && effect == o.effect && autoCycle == o.autoCycle &&
               intervalS == o.intervalS && fadeMs == o.fadeMs && brightness == o.brightness &&
               speed == o.speed && hue == o.hue && cycle == o.cycle && night == o.night &&
               nightFrom == o.nightFrom && nightTo == o.nightTo;
    }
    bool operator!=(const Settings& o) const { return !(*this == o); }
};

enum class ApplyResult { Ok, UnknownKey, BadValue };

namespace detail {

inline bool parseBool(const char* v, bool& out) {
    if (!strcmp(v, "1") || !strcmp(v, "true") || !strcmp(v, "on")) return out = true, true;
    if (!strcmp(v, "0") || !strcmp(v, "false") || !strcmp(v, "off")) return out = false, true;
    return false;
}

// Parses a non-negative decimal integer and clamps it to [lo, hi].
inline bool parseUint(const char* v, long lo, long hi, long& out) {
    if (!v || !*v) return false;
    char* end = nullptr;
    long n = strtol(v, &end, 10);
    if (*end != '\0' || n < 0) return false;
    out = n < lo ? lo : n > hi ? hi : n;
    return true;
}

}  // namespace detail

// Sets one field from its string form (HTTP form parameter). Numbers are
// clamped to their valid range; effect must be < effectCount.
inline ApplyResult apply(Settings& s, const char* key, const char* value, size_t effectCount) {
    using namespace detail;
    enum Id { On, Auto, Night, Effect, Interval, Fade, Brightness, Speed, Hue, Cycle, NightFrom, NightTo };
    struct Field {
        const char* key;
        Id id;
        long lo, hi;  // numbers only
    };
    static const Field fields[] = {
        {"on", On, 0, 0},
        {"auto", Auto, 0, 0},
        {"night", Night, 0, 0},
        {"effect", Effect, 0, 255},
        {"interval", Interval, Settings::INTERVAL_MIN_S, Settings::INTERVAL_MAX_S},
        {"fade", Fade, 0, Settings::FADE_MAX_MS},
        {"brightness", Brightness, Settings::BRIGHTNESS_MIN, 255},
        {"speed", Speed, 0, 255},
        {"hue", Hue, 0, 255},
        {"cycle", Cycle, 0, 0x7FFFFFFF},
        {"night_from", NightFrom, 0, 1439},
        {"night_to", NightTo, 0, 1439},
    };
    for (const Field& f : fields) {
        if (strcmp(key, f.key)) continue;
        if (f.id <= Night) {
            bool b = false;
            if (!parseBool(value, b)) return ApplyResult::BadValue;
            (f.id == On ? s.on : f.id == Auto ? s.autoCycle : s.night) = b;
            return ApplyResult::Ok;
        }
        long n = 0;
        if (!parseUint(value, f.lo, f.hi, n)) return ApplyResult::BadValue;
        switch (f.id) {
            case Effect:
                if (size_t(n) >= effectCount) return ApplyResult::BadValue;
                s.effect = uint8_t(n);
                break;
            case Interval: s.intervalS = uint16_t(n); break;
            case Fade: s.fadeMs = uint16_t(n); break;
            case Brightness: s.brightness = uint8_t(n); break;
            case Speed: s.speed = uint8_t(n); break;
            case Hue: s.hue = uint8_t(n); break;
            case Cycle: s.cycle = uint32_t(n); break;
            case NightFrom: s.nightFrom = uint16_t(n); break;
            case NightTo: s.nightTo = uint16_t(n); break;
            default: break;
        }
        return ApplyResult::Ok;
    }
    return ApplyResult::UnknownKey;
}

// True if minuteOfDay (0..1439) falls in the night window [from, to), which may wrap
// past midnight (from > to). from == to means no night.
inline bool isNight(uint16_t minuteOfDay, uint16_t from, uint16_t to) {
    if (from == to) return false;
    if (from < to) return minuteOfDay >= from && minuteOfDay < to;
    return minuteOfDay >= from || minuteOfDay < to;
}

}  // namespace owl

#include <stdio.h>

namespace owl {

// Writes the settings (plus the effect currently shown) as a JSON object.
// Returns the length written, or 0 if buf is too small.
inline size_t toJson(char* buf, size_t len, const Settings& s, size_t currentEffect) {
    int n = snprintf(buf, len,
                     "{\"on\":%s,\"effect\":%u,\"current\":%u,\"auto\":%s,\"interval\":%u,"
                     "\"fade\":%u,\"brightness\":%u,\"speed\":%u,\"hue\":%u}",
                     s.on ? "true" : "false", unsigned(s.effect), unsigned(currentEffect),
                     s.autoCycle ? "true" : "false", unsigned(s.intervalS), unsigned(s.fadeMs),
                     unsigned(s.brightness), unsigned(s.speed), unsigned(s.hue));
    return (n > 0 && size_t(n) < len) ? size_t(n) : 0;
}

}  // namespace owl
