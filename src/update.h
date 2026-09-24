#pragma once
// Firmware updates from GitHub releases (SPEC v2 §Firmware updates).
// Sprint 05: only the configured project ("owner/repo", NVS "update").

#include <Arduino.h>

#include "owl/boot_status.h"

namespace owl::update {

void begin();
String project();                      // "" if not configured
bool setProject(const char* urlOrRef);  // false if not a GitHub project reference

void loop();  // reports installer progress as events, restarts after a successful install
// Installs an image from explicit URLs (debug hook; GitHub lookup comes in T-43).
bool installFrom(const String& imageUrl, const String& sigUrl);
void setEventSink(void (*sink)(const char* json));
// Checks now (WiFi is switched on for it) and installs a newer signed release automatically.
// Reports {"type":"update_info",...} then update progress. False if busy or not configured.
bool check();
UpdateStatus bootStatus();  // for the boot status display
bool installing();
uint8_t installPercent();

}  // namespace owl::update
