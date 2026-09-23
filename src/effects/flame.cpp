#include "effects/effects.h"
#include "effects/palette.h"

namespace owl::effects {

// Fire2012-style flame, one heat column per LED column, starting at each
// column's own bottom LED so ears/feet burn from their base. Simulation runs
// at a fixed step so it looks the same at any frame rate.
class Flame : public Effect {
public:
    const char* name() const override { return "flame"; }

    void start() override {
        memset(heat_, 0, sizeof(heat_));
        acc_ = 0;
    }

    void render(Canvas& c, const Frame& f) override {
        // acc_ is in 1/100 ms so the speed factor keeps fractional time
        for (acc_ += f.dt * SPEED_PCT; acc_ >= STEP_MS * 100; acc_ -= STEP_MS * 100) step();
        c.fill(CRGB::Black);
        for (int x = 0; x < W; ++x) {
            int col = W - 1 - x;  // grid x -> physical column
            const Column& cl = config::COLUMNS[col];
            for (int k = 0; k < cl.count; ++k)
                c.set(x, cl.yOffset + k, ColorFromPalette(HeatColors_p, scale8(heat_[x][k], 240)));
        }
    }

private:
    static constexpr int W = leds::LAYOUT.width;
    static constexpr int H = leds::LAYOUT.height;
    static constexpr uint32_t STEP_MS = 30;
    static constexpr uint32_t SPEED_PCT = 35;  // flame runs at 0.35x the global speed
    static constexpr uint8_t COOLING = 70;   // higher = shorter flames
    static constexpr uint8_t SPARKING = 110; // higher = more vigorous

    void step() {
        for (int x = 0; x < W; ++x) {
            int n = config::COLUMNS[W - 1 - x].count;
            uint8_t* h = heat_[x];
            for (int k = 0; k < n; ++k) h[k] = qsub8(h[k], random8(0, ((COOLING * 10) / n) + 2));
            for (int k = n - 1; k >= 2; --k) h[k] = (h[k - 1] + h[k - 2] + h[k - 2]) / 3;
            if (random8() < SPARKING) {
                int k = random8(n < 3 ? n : 3);
                h[k] = qadd8(h[k], random8(160, 255));
            }
        }
    }

    uint8_t heat_[W][H];
    uint32_t acc_ = 0;
};

Effect& flame() {
    static Flame e;
    return e;
}

}  // namespace owl::effects
