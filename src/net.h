#pragma once
// WiFi: home network (STA) with captive-portal fallback, mDNS. Non-blocking.

namespace owl::net {

void begin();
void loop();
bool online();  // connected to the home network

}  // namespace owl::net
