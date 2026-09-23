#pragma once
// Hardware-independent timing helpers (unit-tested on host).

#include <stdint.h>

namespace owl {

// Fixed-rate frame pacing. due() returns true once per period, skipping
// missed frames instead of bursting to catch up. Wrap-safe on millis().
class FramePacer {
public:
    explicit FramePacer(uint32_t periodMs) : period_(periodMs) {}

    bool due(uint32_t nowMs) {
        if (!started_) {
            started_ = true;
            next_ = nowMs + period_;
            return true;
        }
        if (int32_t(nowMs - next_) < 0) return false;
        next_ += period_;
        if (int32_t(nowMs - next_) >= 0) next_ = nowMs + period_;  // fell behind
        return true;
    }

private:
    uint32_t period_;
    uint32_t next_ = 0;
    bool started_ = false;
};

// Boot test pattern: lights strip indices 0..numLeds-1 one after another.
// Returns the lit index at elapsedMs, or -1 once the walk is finished.
inline int32_t walkIndex(uint32_t elapsedMs, uint32_t stepMs, uint16_t numLeds) {
    uint32_t i = elapsedMs / stepMs;
    return i < numLeds ? int32_t(i) : -1;
}

}  // namespace owl
