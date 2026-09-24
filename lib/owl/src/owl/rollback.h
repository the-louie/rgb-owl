#pragma once
// When a freshly installed image counts as good (SPEC v2: BLE advertising + 60 s up).
// Hardware-independent; unit-tested on host.

#include <stdint.h>

namespace owl {

inline bool shouldMarkValid(bool bleUp, uint32_t uptimeMs, uint32_t graceMs) {
    return bleUp && uptimeMs >= graceMs;
}

}  // namespace owl
