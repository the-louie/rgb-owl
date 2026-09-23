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
// Measured 2026-09-23 (62 LEDs, grid 6x12). Tops of columns 0-4 align;
// column 5 starts at the bottom row. Eye positions are provisional.

#include "owl/layout_map.h"

namespace owl::config {

constexpr Column COLUMNS[] = {
    {10, 2},  // col 0 (right):  LEDs  0- 9, up
    {11, 1},  // col 1:          LEDs 10-20, down
    {11, 1},  // col 2:          LEDs 21-31, up
    {12, 0},  // col 3:          LEDs 32-43, down
    {11, 1},  // col 4:          LEDs 44-54, up
    {7, 0},   // col 5 (left):   LEDs 55-61, down
};

// right eye: LEDs 6, 13; left eye: LEDs 35, 51 (all on grid row 8)
constexpr ColumnLed EYES[] = {
    {0, 6}, {1, 7}, {3, 8}, {4, 7},
};

OWL_DEFINE_LAYOUT(COLUMNS);

}  // namespace owl::config
