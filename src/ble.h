#pragma once
// Bluetooth LE control (NimBLE GATT server "Owl"). Bonding with the static
// passkey OWL_BLE_PIN is required for every characteristic. See SPEC.md v2 §BLE.

#include <stdint.h>

namespace owl::ble {

// Service and characteristic UUIDs (shared with the Android app).
constexpr const char* SERVICE_UUID = "4f574c00-8a1b-4c2e-9d3f-2b1a6c7e0001";
constexpr const char* STATE_UUID = "4f574c00-8a1b-4c2e-9d3f-2b1a6c7e0002";    // read, notify: JSON
constexpr const char* COMMAND_UUID = "4f574c00-8a1b-4c2e-9d3f-2b1a6c7e0003";  // write: "verb k=v&..."
constexpr const char* EVENT_UUID = "4f574c00-8a1b-4c2e-9d3f-2b1a6c7e0004";    // notify: JSON replies
// Advertised manufacturer data: company 0xFFFF (reserved for testing) + 1 byte, 1 = new phones may
// pair now (first PAIRING_WINDOW_MS after boot), 0 = only bonded phones. The app shows it in its scan list.
constexpr uint16_t ADV_COMPANY_ID = 0xFFFF;

constexpr const char* EFFECTS_UUID = "4f574c00-8a1b-4c2e-9d3f-2b1a6c7e0005";  // read: JSON array

void begin();
int bondCount();
bool advertising();  // GATT server started and advertising
void loop();  // runs queued commands and pushes state changes (main loop only)
// Runs one command line as if written to the command characteristic (debug: POST /api/cmd).
// Events are also logged while debug mode is on.
void execute(const char* line);

}  // namespace owl::ble
