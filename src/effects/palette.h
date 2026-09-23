#pragma once
// Shared colour constants and helpers for effects.

#include <FastLED.h>

namespace owl {

constexpr uint8_t PASTEL_SAT = 120;  // pastel: low saturation, full value

inline CHSV pastel(uint8_t hue, uint8_t val = 255) { return CHSV(hue, PASTEL_SAT, val); }

// Per-frame fade amount for an exponential decay with the given half-life.
inline uint8_t fadeFor(uint32_t dtMs, uint32_t halfLifeMs) {
    uint32_t a = dtMs * 128 / (halfLifeMs ? halfLifeMs : 1);
    return a > 255 ? 255 : uint8_t(a);
}

}  // namespace owl
