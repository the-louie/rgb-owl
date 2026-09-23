#include "effect.h"

namespace owl {

// Placeholder until the first real effect lands: slow pastel hue drift.
class HueDrift : public Effect {
public:
    const char* name() const override { return "drift"; }
    void render(Canvas& c, const Frame& f) override { c.fill(CHSV(uint8_t(f.t / 64), 90, 255)); }
};

static HueDrift drift;

static Effect* const ALL[] = {&drift};
const Registry<Effect> EFFECTS(ALL);

}  // namespace owl
