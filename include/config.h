#pragma once
// Hardware and runtime constants. See SPEC.md / AGENTS.md before changing.

#include <stdint.h>

namespace owl::config {

constexpr uint8_t LED_PIN = 1;              // GPIO1, WS2812B DIN (3.3 V direct)
constexpr uint8_t LED_VOLTS = 5;
constexpr uint32_t LED_MAX_MILLIAMPS = 8000;  // 5 V 10 A PSU, 20 % headroom
constexpr uint16_t FPS = 60;
constexpr uint16_t WALK_STEP_MS = 40;       // boot column-walk test pattern
constexpr uint32_t SAVE_DELAY_MS = 5000;    // NVS write after settings are stable this long
// User-adjustable defaults (brightness, cycle interval, fade, ...) live in lib/owl/src/owl/settings.h.

constexpr const char* HOSTNAME = "owl";        // DHCP hostname + mDNS: owl.local
constexpr const char* SETUP_AP = "Owl-Setup";  // captive-portal access point (open)
constexpr uint32_t WIFI_CONNECT_MS = 15000;    // home WiFi attempt before opening the portal
constexpr uint32_t PORTAL_MS = 300000;         // portal lifetime before retrying home WiFi

}  // namespace owl::config
