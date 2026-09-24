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
// Saves only if exactly these credentials passed the last startTest(); false otherwise.
bool saveTested(const char* ssid, const char* pass);
void forgetCredentials();
// Asynchronous jobs; results are JSON events passed to the sink (the BLE event characteristic):
//   scan -> {"type":"wifi_net","ssid","rssi","secure"} per network, then {"type":"wifi_scan_done","count"}
//   test -> {"type":"wifi_test","ok":true,"rssi"} or {"type":"wifi_test","ok":false,"msg"}
// Return false if another job is running.
bool startScan();
bool startTest(const char* ssid, const char* pass);
void setEventSink(void (*sink)(const char* json));
const char* statusName();
String ssidName();

// Appends WiFi debug fields ("key":value,...) to a JSON object body.
void appendDebug(String& json);

}  // namespace owl::net
