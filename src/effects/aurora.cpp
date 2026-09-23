#include "effects/effects.h"
#include "effects/palette.h"

namespace owl::effects {

// Northern lights: soft noise curtains drifting sideways, pastel green
// through blue to violet, over a faint dark-blue sky.
class Aurora : public Effect {
public:
    const char* name() const override { return "aurora"; }

    void render(Canvas& c, const Frame& f) override {
        const auto& L = leds::LAYOUT;
        uint32_t t = f.t;
        for (int x = 0; x < L.width; ++x) {
            for (int y = 0; y < L.height; ++y) {
                uint8_t band = inoise8(uint16_t(x * 70 + t / 5), uint16_t(y * 22), uint16_t(t / 9));
                uint8_t v = ease8InOutQuad(qsub8(band, 70) * 255 / 185);
                uint8_t hue = 96 + (inoise8(uint16_t(x * 35 + 5000), uint16_t(y * 18 + t / 7)) >> 1);
                CRGB px = CRGB(pastel(hue, v));
                px += CRGB(0, 0, 6);  // sky
                c.set(x, y, px);
            }
        }
    }
};

Effect& aurora() {
    static Aurora e;
    return e;
}

}  // namespace owl::effects
