#pragma once
// Eye blink envelope: open for a gap, then close (80 ms), stay shut (100 ms),
// open (120 ms). Hardware-independent; unit-tested on host.

#include <stdint.h>

namespace owl {

class Blink {
public:
    static constexpr uint32_t CLOSING_MS = 80;
    static constexpr uint32_t CLOSED_MS = 100;
    static constexpr uint32_t OPENING_MS = 120;
    static constexpr uint32_t BLINK_MS = CLOSING_MS + CLOSED_MS + OPENING_MS;

    explicit Blink(uint32_t firstGapMs) : gap_(firstGapMs) {}

    // Advances by dtMs and returns eye openness (255 = open, 0 = shut).
    // nextGapMs is used as the open time after the current blink finishes.
    uint8_t update(uint32_t dtMs, uint32_t nextGapMs) {
        t_ += dtMs;
        while (t_ >= gap_ + BLINK_MS) {
            t_ -= gap_ + BLINK_MS;
            gap_ = nextGapMs;
        }
        if (t_ < gap_) return 255;
        uint32_t b = t_ - gap_;
        if (b < CLOSING_MS) return uint8_t(255 - b * 255 / CLOSING_MS);
        b -= CLOSING_MS;
        if (b < CLOSED_MS) return 0;
        b -= CLOSED_MS;
        return uint8_t(b * 255 / OPENING_MS);
    }

private:
    uint32_t gap_;
    uint32_t t_ = 0;
};

}  // namespace owl
