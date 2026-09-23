#pragma once
// Frame statistics over 1 s windows: frames per second and slowest frame.
// Hardware-independent; unit-tested on host.

#include <stdint.h>

namespace owl {

class FrameStats {
public:
    // Records one frame that took frameUs, finished at nowMs.
    void record(uint32_t frameUs, uint32_t nowMs) {
        if (!started_) {
            started_ = true;
            windowStart_ = nowMs;
        }
        ++frames_;
        if (frameUs > maxUs_) maxUs_ = frameUs;
        if (nowMs - windowStart_ >= 1000) {
            fps_ = frames_ * 1000 / (nowMs - windowStart_);
            lastMaxUs_ = maxUs_;
            frames_ = 0;
            maxUs_ = 0;
            windowStart_ = nowMs;
        }
    }

    uint32_t fps() const { return fps_; }            // last complete window
    uint32_t maxFrameUs() const { return lastMaxUs_; }

private:
    bool started_ = false;
    uint32_t windowStart_ = 0;
    uint32_t frames_ = 0;
    uint32_t maxUs_ = 0;
    uint32_t fps_ = 0;
    uint32_t lastMaxUs_ = 0;
};

}  // namespace owl
