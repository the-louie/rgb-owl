#include "effects/effects.h"

namespace owl {

// Auto-cycle order.
static Effect* const ALL[] = {
    &effects::plasma(),
    &effects::rain(),
    &effects::flame(),
    &effects::aurora(),
    &effects::rainbow(),
    &effects::breathing(),
};
const Registry<Effect> EFFECTS(ALL);

}  // namespace owl
