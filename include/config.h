#pragma once
// Hardware and runtime constants. See SPEC.md / AGENTS.md before changing.

#include <stdint.h>

namespace owl::config {

constexpr uint8_t LED_PIN = 1;              // GPIO1, WS2812B DIN (3.3 V direct)
constexpr uint8_t LED_VOLTS = 5;
constexpr uint32_t LED_MAX_MILLIAMPS = 8000;  // 5 V 10 A PSU, 20 % headroom
constexpr uint8_t DEFAULT_BRIGHTNESS = 128;
constexpr uint16_t FPS = 60;
constexpr uint16_t WALK_STEP_MS = 40;       // boot column-walk test pattern
constexpr uint32_t CYCLE_INTERVAL_MS = 60000;  // auto-cycle: time per effect
constexpr uint32_t CYCLE_FADE_MS = 2000;       // auto-cycle: crossfade duration

}  // namespace owl::config
