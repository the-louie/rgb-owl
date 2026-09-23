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

    static constexpr uint16_t INTERVAL_MIN_S = 5;
    static constexpr uint16_t INTERVAL_MAX_S = 3600;
    static constexpr uint16_t FADE_MAX_MS = 10000;
    static constexpr uint8_t BRIGHTNESS_MIN = 1;

    bool operator==(const Settings& o) const {
        return on == o.on && effect == o.effect && autoCycle == o.autoCycle &&
               intervalS == o.intervalS && fadeMs == o.fadeMs && brightness == o.brightness &&
               speed == o.speed && hue == o.hue;
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
    long n = 0;
    bool b = false;
    if (!strcmp(key, "on") || !strcmp(key, "auto")) {
        if (!parseBool(value, b)) return ApplyResult::BadValue;
        (key[0] == 'o' ? s.on : s.autoCycle) = b;
        return ApplyResult::Ok;
    }
    struct Field {
        const char* key;
        long lo, hi;
    };
    static const Field fields[] = {
        {"effect", 0, 255},
        {"interval", Settings::INTERVAL_MIN_S, Settings::INTERVAL_MAX_S},
        {"fade", 0, Settings::FADE_MAX_MS},
        {"brightness", Settings::BRIGHTNESS_MIN, 255},
        {"speed", 0, 255},
        {"hue", 0, 255},
    };
    for (const Field& f : fields) {
        if (strcmp(key, f.key)) continue;
        if (!parseUint(value, f.lo, f.hi, n)) return ApplyResult::BadValue;
        switch (f.key[0]) {
            case 'e':
                if (size_t(n) >= effectCount) return ApplyResult::BadValue;
                s.effect = uint8_t(n);
                break;
            case 'i': s.intervalS = uint16_t(n); break;
            case 'f': s.fadeMs = uint16_t(n); break;
            case 'b': s.brightness = uint8_t(n); break;
            case 's': s.speed = uint8_t(n); break;
            case 'h': s.hue = uint8_t(n); break;
        }
        return ApplyResult::Ok;
    }
    return ApplyResult::UnknownKey;
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
