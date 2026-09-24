#pragma once
// Firmware updates from GitHub releases (SPEC v2 §Firmware updates).
// Sprint 05: only the configured project ("owner/repo", NVS "update").

#include <Arduino.h>

namespace owl::update {

void begin();
String project();                      // "" if not configured
bool setProject(const char* urlOrRef);  // false if not a GitHub project reference

}  // namespace owl::update
