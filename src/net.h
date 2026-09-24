#pragma once
// WiFi v2 (SPEC v2 §WiFi): credentials come from the app (NVS "wifi"); WiFi is on
// for the boot window and in debug mode only. Non-blocking.

#include <Arduino.h>

#include "owl/wifi_policy.h"

namespace owl::net {

void begin();
void loop();
bool online();                    // connected to the home network
bool configured();                // credentials stored
bool windowOpen();                // 3-minute post-boot window
WifiPolicy::Status status();
void setDevmode(bool on);
bool devmode();
void saveCredentials(const char* ssid, const char* pass);
void forgetCredentials();
// Appends WiFi debug fields ("key":value,...) to a JSON object body.
void appendDebug(String& json);

}  // namespace owl::net
