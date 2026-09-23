#include "effects/effects.h"

namespace owl {

// Auto-cycle order (SPEC.md §Effects).
static Effect* const ALL[] = {
    &effects::plasma(),
    &effects::rain(),
    &effects::flame(),
    &effects::aurora(),
    &effects::eyes(),
    &effects::rainbow(),
    &effects::breathing(),
};
const Registry<Effect> EFFECTS(ALL);

}  // namespace owl
