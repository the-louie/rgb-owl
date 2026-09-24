#pragma once
// Downloads a firmware image + its signature and installs it into the other OTA slot.
// Runs in its own task so LEDs and BLE keep going; poll state() from the main loop.

#include <Arduino.h>

namespace owl::installer {

enum class State { Idle, Downloading, Verifying, Done, Failed };

// http(s) URLs; HTTPS is checked against the embedded CA bundle. False if already running.
bool start(const String& imageUrl, const String& sigUrl);
State state();
uint8_t percent();
const char* error();  // set when Failed
void reset();         // Failed -> Idle

}  // namespace owl::installer
