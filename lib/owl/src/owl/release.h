#pragma once
// Picks the release to install from a list of GitHub releases. Hardware-independent; unit-tested.

#include <stddef.h>
#include <string.h>

#include "owl/protocol.h"

namespace owl {

struct ReleaseEntry {
    const char* tag;  // "v1.2.3"
    bool prerelease;
    bool draft;
};

// Index of the highest-versioned eligible release (drafts never; pre-releases only if
// includePre; tags must be semver), or -1.
inline int pickRelease(const ReleaseEntry* r, size_t n, bool includePre) {
    int best = -1;
    SemVer bestV;
    for (size_t i = 0; i < n; ++i) {
        SemVer v;
        if (r[i].draft || (r[i].prerelease && !includePre) || !v.parse(r[i].tag)) continue;
        if (best < 0 || compare(v, bestV) > 0) {
            best = int(i);
            bestV = v;
        }
    }
    return best;
}

enum class Asset { Other, Image, Signature };

// Release asset names: "owl-firmware-1.2.3.bin" (+ ".sig"), or the unversioned
// "owl-firmware.bin" (+ ".sig") that v1.0.0 looks for.
inline Asset assetKind(const char* name) {
    const char* p = "owl-firmware";
    size_t lp = strlen(p), n = strlen(name);
    if (strncmp(name, p, lp) != 0) return Asset::Other;
    const char* rest = name + lp;
    if (*rest != '.' && *rest != '-') return Asset::Other;
    auto ends = [&](const char* suffix) { size_t ls = strlen(suffix); return n >= ls && !strcmp(name + n - ls, suffix); };
    if (ends(".bin.sig")) return Asset::Signature;
    if (ends(".bin") && strncmp(rest, "-s3zero-merged", 14) != 0) return Asset::Image;
    return Asset::Other;
}

// True if tag is a newer version than current (e.g. OWL_VERSION).
inline bool isNewer(const char* tag, const char* current) {
    SemVer t, c;
    if (!t.parse(tag)) return false;
    if (!c.parse(current)) return true;  // unknown current version: allow updating
    return compare(t, c) > 0;
}

}  // namespace owl
