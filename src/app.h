#pragma once
// Owl application: render loop, effect cycling and settings.

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

}  // namespace owl::app
