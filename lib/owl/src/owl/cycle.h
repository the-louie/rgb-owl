#pragma once
// Effect auto-cycle + crossfade scheduler. Hardware-independent; unit-tested on host.
//
// Holds the current effect for intervalMs, then crossfades to the next one over
// fadeMs. mix is the weight of `next` (0..255). `started` reports an effect
// that became visible in this update so the caller can call its start().

#include <stddef.h>
#include <stdint.h>

namespace owl {

class Cycler {
public:
    struct State {
        size_t current;
        int next;     // -1 when not fading
        uint8_t mix;  // weight of next, 0..255
        int started;  // index that became visible in this update, or -1
    };

    Cycler(size_t count, uint32_t intervalMs, uint32_t fadeMs)
        : count_(count), interval_(intervalMs), fade_(fadeMs) {}

    void setInterval(uint32_t ms) { interval_ = ms; }
    void setFade(uint32_t ms) { fade_ = ms; }
    void setAuto(bool on) { auto_ = on; }
    bool autoCycle() const { return auto_; }
    size_t current() const { return current_; }

    // Crossfade to effect i (finishing any fade in progress first).
    void select(size_t i) {
        if (i >= count_) return;
        if (next_ >= 0) finish();
        if (i == current_) return;
        begin(i);
    }

    State update(uint32_t dtMs) {
        started_ = -1;
        if (next_ < 0) {
            elapsed_ += dtMs;
            if (auto_ && count_ > 1 && elapsed_ >= interval_) begin((current_ + 1) % count_);
        } else {
            fadeT_ += dtMs;
        }
        if (next_ >= 0 && fadeT_ >= fade_) finish();
        uint8_t mix = next_ < 0 ? 0 : uint8_t(fadeT_ * 255 / fade_);
        return State{current_, next_, mix, started_};
    }

private:
    void begin(size_t i) {
        next_ = int(i);
        fadeT_ = 0;
        started_ = int(i);
    }
    void finish() {
        current_ = size_t(next_);
        next_ = -1;
        elapsed_ = 0;
    }

    size_t count_;
    uint32_t interval_;
    uint32_t fade_;
    bool auto_ = true;
    size_t current_ = 0;
    int next_ = -1;
    uint32_t elapsed_ = 0;
    uint32_t fadeT_ = 0;
    int started_ = -1;
};

}  // namespace owl
