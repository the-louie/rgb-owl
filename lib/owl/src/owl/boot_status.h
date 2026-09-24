#pragma once
// Boot status display as whole-owl phases (SPEC v2 §Boot status). Decides what to show;
// the firmware renders it. Hardware-independent; unit-tested on host.
//   1 BLE:    green = phone(s) bonded, blue = none                       (FLASH_MS)
//   2 WiFi:   red = not configured (skip 3), yellow pulse = connecting,
//             green = connected (FLASH_MS), red/black blink = failed
//   3 Update: purple pulse = checking, green = up to date, cyan fill = downloading, red = failed
// A final result stays up for RESULT_MS, then done() is true and effects start.

#include <stdint.h>

#include "owl/wifi_policy.h"

namespace owl {

enum class UpdateStatus { None, Checking, UpToDate, Downloading, Failed };  // None: no check this boot

struct Visual {
    enum Kind { Solid, Pulse, Blink, Fill } kind;
    uint8_t r, g, b;
    uint8_t fill;  // Fill: 0..100 %
    bool operator==(const Visual& o) const {
        return kind == o.kind && r == o.r && g == o.g && b == o.b && fill == o.fill;
    }
};

class BootStatus {
public:
    static constexpr uint32_t FLASH_MS = 1000;
    static constexpr uint32_t RESULT_MS = 3000;

    static constexpr Visual GREEN{Visual::Solid, 0, 255, 0, 0};
    static constexpr Visual BLUE{Visual::Solid, 0, 0, 255, 0};
    static constexpr Visual RED{Visual::Solid, 255, 0, 0, 0};
    static constexpr Visual YELLOW_PULSE{Visual::Pulse, 255, 180, 0, 0};
    static constexpr Visual RED_BLINK{Visual::Blink, 255, 0, 0, 0};
    static constexpr Visual PURPLE_PULSE{Visual::Pulse, 160, 0, 255, 0};

    void begin(uint32_t nowMs, bool bonded) {
        start_ = nowMs;
        bonded_ = bonded;
        phase_ = Phase::Ble;
        hasResult_ = hasConnected_ = false;
        done_ = false;
    }

    bool done() const { return done_; }

    Visual update(uint32_t nowMs, WifiPolicy::Status wifi, UpdateStatus upd, uint8_t pct) {
        using W = WifiPolicy::Status;
        switch (phase_) {
            case Phase::Ble:
                if (nowMs - start_ < FLASH_MS) return bonded_ ? BLUE_OR_GREEN(true) : BLUE_OR_GREEN(false);
                phase_ = Phase::Wifi;
                start_ = nowMs;
                [[fallthrough]];
            case Phase::Wifi:
                if (wifi == W::Unconfigured) return result(nowMs, RED);
                if (wifi == W::Failed || wifi == W::Off) return result(nowMs, RED_BLINK);
                if (wifi == W::Connecting) return YELLOW_PULSE;
                if (!hasConnected_) {
                    hasConnected_ = true;
                    connectedAt_ = nowMs;
                }
                if (nowMs - connectedAt_ < FLASH_MS) return GREEN;
                phase_ = Phase::Update;
                [[fallthrough]];
            case Phase::Update:
                switch (upd) {
                    case UpdateStatus::None: return result(nowMs, GREEN);
                    case UpdateStatus::Checking: return PURPLE_PULSE;
                    case UpdateStatus::UpToDate: return result(nowMs, GREEN);
                    case UpdateStatus::Downloading: return Visual{Visual::Fill, 0, 220, 255, pct > 100 ? uint8_t(100) : pct};
                    case UpdateStatus::Failed: return result(nowMs, RED);
                }
        }
        return RED;
    }

private:
    enum class Phase { Ble, Wifi, Update };

    static constexpr Visual BLUE_OR_GREEN(bool bonded) { return bonded ? GREEN : BLUE; }

    // Shows a final result for RESULT_MS, then marks the sequence done.
    Visual result(uint32_t nowMs, const Visual& v) {
        if (!hasResult_) {
            hasResult_ = true;
            resultAt_ = nowMs;
        }
        if (nowMs - resultAt_ >= RESULT_MS) done_ = true;
        return v;
    }

    Phase phase_ = Phase::Ble;
    uint32_t start_ = 0;
    uint32_t connectedAt_ = 0;
    uint32_t resultAt_ = 0;
    bool hasConnected_ = false;
    bool hasResult_ = false;
    bool bonded_ = false;
    bool done_ = false;
};

}  // namespace owl
