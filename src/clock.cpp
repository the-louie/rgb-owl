#include "clock.h"

#include <Preferences.h>
#include <sys/time.h>
#include <time.h>

#include "log.h"
#include "net.h"

namespace owl::clock {

// Europe/Stockholm until the phone sends its own zone.
constexpr const char* DEFAULT_TZ = "CET-1CEST,M3.5.0,M10.5.0/3";
constexpr time_t MIN_VALID = 1700000000;  // 2023-11: anything earlier means "never set"

static String zone = DEFAULT_TZ;
static bool ntpStarted = false;

static void applyZone() {
    setenv("TZ", zone.c_str(), 1);
    tzset();
}

void begin() {
    Preferences p;
    if (p.begin("clock", true)) {
        zone = p.getString("tz", DEFAULT_TZ);
        p.end();
    }
    applyZone();
}

void loop() {
    if (net::online() && !ntpStarted) {  // SNTP keeps syncing on its own while WiFi is up
        configTzTime(zone.c_str(), "pool.ntp.org", "time.google.com");
        ntpStarted = true;
    }
}

bool valid() { return time(nullptr) > MIN_VALID; }

int minuteOfDay() {
    if (!valid()) return -1;
    time_t now = time(nullptr);
    struct tm t;
    localtime_r(&now, &t);
    return t.tm_hour * 60 + t.tm_min;
}

void set(uint32_t epoch, const char* tz) {
    struct timeval tv = {time_t(epoch), 0};
    settimeofday(&tv, nullptr);
    if (tz && *tz && zone != tz && strlen(tz) < 64) {
        zone = tz;
        Preferences p;
        if (p.begin("clock", false)) {
            p.putString("tz", zone);
            p.end();
        }
        if (ntpStarted) configTzTime(zone.c_str(), "pool.ntp.org", "time.google.com");
    }
    applyZone();
    log::printf("clock: set to %lu, tz %s, local minute %d", (unsigned long)epoch, zone.c_str(), minuteOfDay());
}

String tz() { return zone; }

}  // namespace owl::clock
