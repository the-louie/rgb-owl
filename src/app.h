#pragma once
// Owl application: render loop, effect cycling and settings.

#include <Arduino.h>
#include <stddef.h>

#include "owl/settings.h"

namespace owl::app {

void begin();
void loop();

const Settings& settings();
// Applies one setting (string form, e.g. from HTTP), updates the hardware and
// schedules an NVS save.
ApplyResult set(const char* key, const char* value);
// Effect currently shown (differs from settings().effect while auto-cycling).
size_t currentEffect();

// Debug test patterns (override effects until mode "none"):
//   none | off | solid (r,g,b) | pixel (index = strip index) |
//   column (index = physical column, 0 = rightmost) | row (index = grid y, 0 = bottom) | walk
// Colour defaults to white-ish when r,g,b are all 0. Returns false for a bad mode/index.
bool setTest(const char* mode, uint8_t r, uint8_t g, uint8_t b, int index);
// Estimated LED current (mA) of the last frame, and frames per second.
uint32_t estimatedMilliamps();
uint32_t fps();
// Appends the app's debug fields ("key":value,...) to a JSON object body.
void appendDebug(String& json);

}  // namespace owl::app
