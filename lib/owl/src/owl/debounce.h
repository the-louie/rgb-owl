#pragma once
// Delays a flash write until settings have been stable for delayMs, so a
// slider drag becomes one NVS write. Hardware-independent; unit-tested on host.

#include <stdint.h>

namespace owl {

class SaveDebouncer {
public:
    explicit SaveDebouncer(uint32_t delayMs) : delay_(delayMs) {}

    void changed(uint32_t nowMs) {
        pending_ = true;
        last_ = nowMs;
    }

    // True once, when a change has been stable for delayMs.
    bool due(uint32_t nowMs) {
        if (!pending_ || nowMs - last_ < delay_) return false;
        pending_ = false;
        return true;
    }

    bool pending() const { return pending_; }

private:
    uint32_t delay_;
    uint32_t last_ = 0;
    bool pending_ = false;
};

}  // namespace owl
