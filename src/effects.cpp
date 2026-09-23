#include "effects/effects.h"

namespace owl {

// Auto-cycle order.
static Effect* const ALL[] = {
    &effects::plasma(),
    &effects::rain(),
    &effects::flame(),
};
const Registry<Effect> EFFECTS(ALL);

}  // namespace owl
