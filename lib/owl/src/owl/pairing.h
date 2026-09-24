#pragma once
// New phones may pair only in the first windowMs after boot; bonded phones any time.
// Hardware-independent; unit-tested on host.

#include <stdint.h>

namespace owl {

inline bool pairingAllowed(bool bonded, uint32_t uptimeMs, uint32_t windowMs) {
    return bonded || uptimeMs < windowMs;
}

}  // namespace owl
