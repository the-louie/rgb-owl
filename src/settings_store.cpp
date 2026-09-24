#include "settings_store.h"

#include <Preferences.h>

namespace owl::store {

// Settings is stored as a raw blob. Fields are only ever appended, so an older, shorter
// blob loads as a prefix and the new fields keep their defaults. Bump VERSION (old
// blobs are then ignored) only if existing fields change.
constexpr uint8_t VERSION = 1;

void load(Settings& s) {
    Preferences p;
    if (!p.begin("owl", true)) return;
    Settings tmp;
    size_t len = p.getBytesLength("s");
    if (p.getUChar("ver", 0) == VERSION && len > 0 && len <= sizeof(tmp) && p.getBytes("s", &tmp, len) == len)
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
