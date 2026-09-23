#include "effects/effects.h"
#include "effects/palette.h"

namespace owl::effects {

// Sum of four moving sine fields, mapped to a slowly rotating pastel hue.
class Plasma : public Effect {
public:
    const char* name() const override { return "plasma"; }

    void render(Canvas& c, const Frame& f) override {
        const auto& L = leds::LAYOUT;
        uint32_t t = f.t;
        int cx = L.width / 2;
        for (int x = 0; x < L.width; ++x) {
            for (int y = 0; y < L.height; ++y) {
                uint16_t v = sin8(uint8_t(x * 40 + t / 11))
                           + sin8(uint8_t(y * 18 - t / 17))
                           + sin8(uint8_t(x * 28 + y * 14 + t / 23))
                           + sin8(uint8_t(sqrt16(uint16_t((x - cx) * (x - cx) * 64 + y * y * 16)) * 3 - t / 13));
                c.set(x, y, pastel(uint8_t(v / 4 + t / 90)));
            }
        }
    }
};

Effect& plasma() {
    static Plasma e;
    return e;
}

}  // namespace owl::effects
