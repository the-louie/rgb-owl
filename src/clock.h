#pragma once
// Wall clock for the night schedule: set by the phone over BLE (`time` verb) and by NTP
// while WiFi is up. Timezone is a POSIX TZ string, stored in NVS.

#include <Arduino.h>

namespace owl::clock {

void begin();
void loop();
bool valid();                 // set at least once since boot
int minuteOfDay();            // local time, 0..1439; -1 if not valid
void set(uint32_t epoch, const char* tz);  // tz may be empty (keep current)
String tz();

}  // namespace owl::clock
