#pragma once
// Hardware and runtime constants. See SPEC.md / AGENTS.md before changing.

#include <stdint.h>

namespace owl::config {

constexpr uint8_t LED_PIN = 1;              // GPIO1, WS2812B DIN (3.3 V direct)
constexpr uint8_t BOOT_BUTTON_PIN = 0;       // BOOT button (strapping pin; read-only use at runtime)
constexpr uint32_t RESET_HOLD_MS = 5000;     // hold BOOT this long: forget bonds + WiFi
constexpr uint8_t LED_VOLTS = 5;
constexpr uint32_t LED_MAX_MILLIAMPS = 4000;  // 62 LEDs x 60 mA = 3.7 A; protects wiring if count/PSU change
constexpr uint32_t COLOR_CORRECTION = 0xFFB0F0;  // FastLED TypicalLEDStrip; tune via POST /api/color
constexpr float GAMMA = 1.0f;                    // output gamma; 1.0 = off; tune via POST /api/color
constexpr uint16_t FPS = 60;
constexpr uint16_t WALK_STEP_MS = 40;       // boot column-walk test pattern
constexpr uint32_t SAVE_DELAY_MS = 5000;    // NVS write after settings are stable this long
// User-adjustable defaults (brightness, cycle interval, fade, ...) live in lib/owl/src/owl/settings.h.

constexpr const char* HOSTNAME = "owl";        // DHCP hostname + mDNS: owl.local
constexpr uint32_t WIFI_CONNECT_MS = 15000;    // give up joining WiFi after this (boot / test)
constexpr uint32_t WIFI_WINDOW_MS = 180000;    // WiFi stays up this long after the boot connection
constexpr uint32_t ROLLBACK_GRACE_MS = 60000;  // new OTA image must run this long (BLE up) to be kept
constexpr uint32_t PAIRING_WINDOW_MS = 180000; // new phones may pair only this long after boot

}  // namespace owl::config
