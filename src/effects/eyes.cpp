#include "effects/effects.h"
#include "effects/palette.h"
#include "owl/blink.h"

namespace owl::effects {

// Owl eyes: warm glowing eyes (EYES[] in layout.h) that blink every 3-8 s,
// over a dim, slowly shifting pastel night body.
class Eyes : public Effect {
public:
    const char* name() const override { return "eyes"; }

    void render(Canvas& c, const Frame& f) override {
        const auto& L = leds::LAYOUT;
        for (int i = 0; i < L.numLeds; ++i) {
            Point p = L.pos[i];
            uint8_t n = inoise8(uint16_t(p.x * 50), uint16_t(p.y * 30), uint16_t(f.t / 8));
            c.px[i] = pastel(uint8_t(170 + (n >> 3)), 20 + (n >> 3));
        }
        uint8_t open = blink_.update(f.dt, random16(3000, 8000));
        uint8_t glow = 200 + scale8(sin8(uint8_t(f.t / 16)), 55);
        CRGB eye = CRGB(CHSV(35, 150, scale8(glow, open)));
        for (const ColumnLed& e : config::EYES) {
            int16_t i = columnLedIndex(config::COLUMNS, e);
            if (i != NO_LED) c.px[i] = eye;
        }
    }

private:
    Blink blink_{4000};
};

Effect& eyes() {
    static Eyes e;
    return e;
}

}  // namespace owl::effects
