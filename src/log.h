#pragma once
// Logging to USB serial and a RAM ring buffer readable at GET /api/log.

#include <Arduino.h>

namespace owl::log {

void printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
String dump();  // buffered log, oldest line first

}  // namespace owl::log
