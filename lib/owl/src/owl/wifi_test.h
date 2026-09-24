#pragma once
// WiFi credential test bookkeeping (SPEC v2: Save only after a passing Test).
// Hardware-independent; unit-tested on host.

#include <stdint.h>
#include <string.h>

namespace owl {

// Why a join attempt failed, from ESP-IDF wifi_err_reason_t codes.
inline const char* wifiFailReason(int reason) {
    switch (reason) {
        case 201: return "network not found";  // WIFI_REASON_NO_AP_FOUND
        case 15:                                // WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT
        case 202:                               // WIFI_REASON_AUTH_FAIL
        case 204: return "wrong password";      // WIFI_REASON_HANDSHAKE_TIMEOUT
        case 0: return "timeout";
        default: return "connection failed";
    }
}

// Remembers which credentials last passed a test, as a hash (the password is not kept twice).
class TestGate {
public:
    void record(const char* ssid, const char* pass, bool passed) {
        passed_ = passed;
        hash_ = hash(ssid, pass);
    }
    bool allowSave(const char* ssid, const char* pass) const { return passed_ && hash_ == hash(ssid, pass); }
    void clear() { passed_ = false; }

private:
    static uint32_t hash(const char* ssid, const char* pass) {
        uint32_t h = 2166136261u;  // FNV-1a over ssid \0 pass
        for (const char* p = ssid;; ++p) {
            h = (h ^ uint8_t(*p)) * 16777619u;
            if (!*p) break;
        }
        for (const char* p = pass; *p; ++p) h = (h ^ uint8_t(*p)) * 16777619u;
        return h;
    }

    bool passed_ = false;
    uint32_t hash_ = 0;
};

}  // namespace owl
