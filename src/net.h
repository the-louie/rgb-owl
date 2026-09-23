#pragma once
// WiFi: home network (STA) with captive-portal fallback, mDNS. Non-blocking.

#include <Arduino.h>

namespace owl::net {

void begin();
void loop();
bool online();  // connected to the home network
// Appends WiFi debug fields ("key":value,...) to a JSON object body.
void appendDebug(String& json);

}  // namespace owl::net
