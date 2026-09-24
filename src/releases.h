#pragma once
// Looks up the newest GitHub release of the configured project (in its own task).
// Stable releases only, unless includePre (debug mode = test channel).

#include <Arduino.h>

namespace owl::releases {

enum class State { Idle, Checking, Done, Failed };

struct Result {
    String tag;       // "v1.2.3", empty if no eligible release
    String imageUrl;  // owl-firmware.bin asset
    String sigUrl;    // owl-firmware.bin.sig asset
    bool newer = false;
    String error;
};

bool start(const String& repo, bool includePre);  // false if a check is running
State state();
const Result& result();  // valid when Done/Failed
void reset();            // Done/Failed -> Idle

}  // namespace owl::releases
