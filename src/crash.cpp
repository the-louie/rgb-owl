#include "crash.h"

#include <Preferences.h>
#include <esp_system.h>
#include <time.h>

namespace owl::crash {

static char reason[24] = "";
static uint32_t when = 0;
static uint32_t total = 0;

const char* resetReason() {
    switch (esp_reset_reason()) {
        case ESP_RST_POWERON: return "power-on";
        case ESP_RST_SW: return "software";
        case ESP_RST_PANIC: return "panic";
        case ESP_RST_INT_WDT: return "interrupt-watchdog";
        case ESP_RST_TASK_WDT: return "task-watchdog";
        case ESP_RST_WDT: return "watchdog";
        case ESP_RST_BROWNOUT: return "brownout";
        case ESP_RST_DEEPSLEEP: return "deep-sleep";
        case ESP_RST_EXT: return "external";
        default: return "other";
    }
}

static bool abnormal(esp_reset_reason_t r) {
    return r == ESP_RST_PANIC || r == ESP_RST_INT_WDT || r == ESP_RST_TASK_WDT || r == ESP_RST_WDT ||
           r == ESP_RST_BROWNOUT;
}

void begin() {
    Preferences p;
    if (!p.begin("crash", false)) return;
    if (abnormal(esp_reset_reason())) {
        time_t now = time(nullptr);  // the RTC keeps system time across a software/panic reset
        p.putString("reason", resetReason());
        p.putULong("time", now > 1700000000 ? uint32_t(now) : 0);
        p.putULong("count", p.getULong("count", 0) + 1);
    }
    p.getString("reason", reason, sizeof(reason));
    when = p.getULong("time", 0);
    total = p.getULong("count", 0);
    p.end();
}

const char* lastReason() { return reason; }
uint32_t lastTime() { return when; }
uint32_t count() { return total; }

}  // namespace owl::crash
