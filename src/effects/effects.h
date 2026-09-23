#pragma once
// One accessor per effect; src/effects.cpp lists them in auto-cycle order.

#include "effect.h"

namespace owl::effects {

Effect& plasma();
Effect& rain();
Effect& flame();
Effect& aurora();
Effect& rainbow();

}  // namespace owl::effects
