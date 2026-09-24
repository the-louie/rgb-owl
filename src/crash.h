#pragma once
// Remembers the last abnormal reset (panic, watchdog, brownout) in NVS "crash".

#include <Arduino.h>

namespace owl::crash {

void begin();  // call early in setup(): records this boot if it followed a crash
const char* lastReason();  // "" if none recorded
uint32_t lastTime();       // unix time of that boot, 0 if the clock was not set
uint32_t count();          // crashes recorded since the NVS was cleared
const char* resetReason(); // reason for the current boot

}  // namespace owl::crash
