#pragma once
// Long-press detector for a button. Hardware-independent; unit-tested on host.

#include <stdint.h>

namespace owl {

class HoldDetector {
public:
    explicit HoldDetector(uint32_t holdMs) : hold_(holdMs) {}

    // Feed the current button state; returns true once per press, when it has
    // been held for holdMs. Releasing re-arms it.
    bool update(bool pressed, uint32_t nowMs) {
        if (!pressed) {
            down_ = fired_ = false;
            return false;
        }
        if (!down_) {
            down_ = true;
            since_ = nowMs;
        }
        if (!fired_ && nowMs - since_ >= hold_) {
            fired_ = true;
            return true;
        }
        return false;
    }

    // ms the button has been held (0 when released), e.g. for progress feedback.
    uint32_t heldMs(uint32_t nowMs) const { return down_ ? nowMs - since_ : 0; }

private:
    uint32_t hold_;
    uint32_t since_ = 0;
    bool down_ = false;
    bool fired_ = false;
};

}  // namespace owl
