#include "effects/effects.h"
#include "effects/palette.h"

namespace owl::effects {

// Calm night light: one colour slowly breathing (5 s period). Full saturation so the default
// hue 160 is the Bodforss brand blue (#040D81) rather than a pastel.
class Breathing : public Effect {
public:
    const char* name() const override { return "breathing"; }

    void render(Canvas& c, const Frame& f) override {
        uint8_t phase = uint8_t((f.t % PERIOD_MS) * 256 / PERIOD_MS);
        uint8_t v = 40 + scale8(cubicwave8(phase), 255 - 40);
        c.fill(CHSV(hue, 245, v));
    }

    uint8_t hue = 160;  // set from Settings::hue (160 = blue)

private:
    static constexpr uint32_t PERIOD_MS = 5000;
};

static Breathing instance;

Effect& breathing() { return instance; }

void setBreathingHue(uint8_t hue) { instance.hue = hue; }

}  // namespace owl::effects
