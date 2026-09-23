#pragma once
// Settings persistence in NVS (Preferences namespace "owl").

#include "owl/settings.h"

namespace owl::store {

// Loads saved settings; leaves defaults if nothing valid is stored.
void load(Settings& s);
void save(const Settings& s);

}  // namespace owl::store
