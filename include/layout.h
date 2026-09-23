#pragma once
// Physical LED layout of the owl. EDIT THIS FILE once the strip is placed.
//
// COLUMNS: listed from the RIGHTMOST column to the leftmost (as seen from the front).
//   The strip starts at the bottom of the rightmost column and runs up, then down
//   the next column, and so on (zig-zag).
//   {count, yOffset}: number of LEDs in the column, and how many LED pitches its
//   lowest LED sits above the lowest LED of the whole owl.
// EYES: LEDs behind the eyes, as {column, row}; column as in COLUMNS (0 = rightmost),
//   row counted from the column's lowest LED (0 = bottom).
//
// Current values are PLACEHOLDERS.

#include "owl/layout_map.h"

namespace owl::config {

constexpr Column COLUMNS[] = {
    {8, 3}, {13, 1}, {15, 0}, {15, 0}, {13, 1}, {8, 3},
};

constexpr ColumnLed EYES[] = {
    {2, 11}, {3, 11},
};

OWL_DEFINE_LAYOUT(COLUMNS);

}  // namespace owl::config
