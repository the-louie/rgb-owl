#include "settings_store.h"

#include <Preferences.h>

namespace owl::store {

// Bump when Settings changes layout; old blobs are then ignored.
constexpr uint8_t VERSION = 1;

void load(Settings& s) {
    Preferences p;
    if (!p.begin("owl", true)) return;
    Settings tmp;
    if (p.getUChar("ver", 0) == VERSION && p.getBytesLength("s") == sizeof(tmp) &&
        p.getBytes("s", &tmp, sizeof(tmp)) == sizeof(tmp))
        s = tmp;
    p.end();
}

void save(const Settings& s) {
    Preferences p;
    if (!p.begin("owl", false)) return;
    p.putBytes("s", &s, sizeof(s));
    p.putUChar("ver", VERSION);
    p.end();
}

}  // namespace owl::store
