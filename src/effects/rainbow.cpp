#include "effects/effects.h"
#include "effects/palette.h"

namespace owl::effects {

// Diagonal pastel rainbow sweep with white twinkles that pop in and fade out.
class Rainbow : public Effect {
public:
    const char* name() const override { return "rainbow"; }

    void start() override {
        memset(sparkle_, 0, sizeof(sparkle_));
        acc_ = 0;
    }

    void render(Canvas& c, const Frame& f) override {
        const auto& L = leds::LAYOUT;
        for (int i = 0; i < N; ++i) {
            Point p = L.pos[i];
            c.px[i] = pastel(uint8_t(p.x * 20 + p.y * 8 - f.t / 12), 170);
        }
        uint8_t fade = fadeFor(f.dt, 150);
        for (int i = 0; i < N; ++i) sparkle_[i] = scale8(sparkle_[i], 255 - fade);
        for (acc_ += f.dt; acc_ >= SPARK_EVERY_MS; acc_ -= SPARK_EVERY_MS) sparkle_[random16(N)] = 255;
        for (int i = 0; i < N; ++i)
            if (sparkle_[i]) c.px[i] = blend(c.px[i], CRGB::White, sparkle_[i]);
    }

private:
    static constexpr int N = leds::LAYOUT.numLeds;
    static constexpr uint32_t SPARK_EVERY_MS = 120;  // ~8 twinkles/s

    uint8_t sparkle_[N];
    uint32_t acc_ = 0;
};

Effect& rainbow() {
    static Rainbow e;
    return e;
}

}  // namespace owl::effects
