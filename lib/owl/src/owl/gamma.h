#pragma once
// Output gamma lookup table (effects work in perceptual values; LEDs are linear).
// Hardware-independent; unit-tested on host.

#include <math.h>
#include <stdint.h>

namespace owl {

class GammaLut {
public:
    explicit GammaLut(float gamma = 1.0f) { set(gamma); }

    // gamma 1.0 = identity. Non-zero inputs never map to 0, so dim pixels stay lit.
    void set(float gamma) {
        gamma_ = gamma;
        for (int i = 0; i < 256; ++i) {
            float v = powf(i / 255.0f, gamma) * 255.0f + 0.5f;
            uint8_t o = v > 255.0f ? 255 : uint8_t(v);
            lut_[i] = (i > 0 && o == 0) ? 1 : o;
        }
    }
    float gamma() const { return gamma_; }
    uint8_t operator[](uint8_t v) const { return lut_[v]; }

private:
    float gamma_ = 1.0f;
    uint8_t lut_[256];
};

}  // namespace owl
