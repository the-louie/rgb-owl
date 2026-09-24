#pragma once
// Firmware updates from GitHub releases (SPEC v2 §Firmware updates).
// Sprint 05: only the configured project ("owner/repo", NVS "update").

#include <Arduino.h>

namespace owl::update {

void begin();
String project();                      // "" if not configured
bool setProject(const char* urlOrRef);  // false if not a GitHub project reference

void loop();  // reports installer progress as events, restarts after a successful install
// Installs an image from explicit URLs (debug hook; GitHub lookup comes in T-43).
bool installFrom(const String& imageUrl, const String& sigUrl);
void setEventSink(void (*sink)(const char* json));
// Starts a release lookup; reports {"type":"update_info",...}. False if busy/no project/offline.
bool check();
bool installing();
uint8_t installPercent();

}  // namespace owl::update
