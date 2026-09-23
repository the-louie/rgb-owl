#include "effects/effects.h"
#include "effects/palette.h"

namespace owl::effects {

// Matrix-style green rain: one drop per column falling at its own speed,
// leaving a fading green trail. Keeps its own trail buffer between frames.
class Rain : public Effect {
public:
    const char* name() const override { return "rain"; }

    void start() override {
        fill_solid(trail_, N, CRGB::Black);
        for (int x = 0; x < W; ++x) spawn(x, true);
    }

    void render(Canvas& c, const Frame& f) override {
        fadeToBlackBy(trail_, N, fadeFor(f.dt, 180));
        for (int x = 0; x < W; ++x) {
            Drop& d = drops_[x];
            if (d.wait > f.dt) {
                d.wait -= f.dt;
                continue;
            }
            d.wait = 0;
            d.y -= int32_t(d.speed * f.dt);  // 1/1000 rows per ms per speed unit
            int y = d.y >> 10;
            if (y < 0) {
                spawn(x, false);
                continue;
            }
            int16_t i = leds::LAYOUT.index(x, y);
            if (i != NO_LED) trail_[i] = CRGB(40, 255, 60);
        }
        for (int i = 0; i < N; ++i) c.px[i] = trail_[i];
        // bright pale head on top of the trail
        for (int x = 0; x < W; ++x)
            if (!drops_[x].wait) c.set(x, drops_[x].y >> 10, CRGB(190, 255, 190));
    }

private:
    static constexpr int W = leds::LAYOUT.width;
    static constexpr int H = leds::LAYOUT.height;
    static constexpr int N = leds::LAYOUT.numLeds;

    struct Drop {
        int32_t y;       // row << 10
        uint16_t speed;  // (rows << 10) per 1000 ms ~= rows/s
        uint32_t wait;   // ms before the drop appears
    };

    void spawn(int x, bool initial) {
        Drop& d = drops_[x];
        d.y = (H - 1) << 10;
        d.speed = random16(6, 16);  // 6..15 rows/s
        d.wait = initial ? random16(0, 1500) : random16(100, 1800);
    }

    CRGB trail_[N];
    Drop drops_[W];
};

Effect& rain() {
    static Rain e;
    return e;
}

}  // namespace owl::effects
