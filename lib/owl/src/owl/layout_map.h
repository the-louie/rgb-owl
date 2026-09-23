#pragma once
// Compile-time mapping between the virtual XY grid and the physical strip index.
//
// Physical model (see AGENTS.md "LED layout model"):
//   - columns are listed right -> left; column 0 is the rightmost one
//   - strip starts at the bottom of column 0 and runs up, column 1 runs down, ...
//   - each column has a LED count and a yOffset (grid row of its lowest LED)
// Grid model used by effects:
//   - x = 0 is the leftmost column, y = 0 is the bottom row
//   - cells without a LED map to NO_LED

#include <stddef.h>
#include <stdint.h>

namespace owl {

struct Column {
    uint8_t count;    // LEDs in this column
    uint8_t yOffset;  // grid row of the column's lowest LED
};

// A LED addressed by physical column (0 = rightmost) and row within that
// column counted from its lowest LED (0 = bottom).
struct ColumnLed {
    uint8_t column;
    uint8_t row;
};

struct Point {
    uint8_t x;
    uint8_t y;
};

constexpr int16_t NO_LED = -1;

template <size_t C>
constexpr size_t totalLeds(const Column (&cols)[C]) {
    size_t n = 0;
    for (size_t i = 0; i < C; ++i) n += cols[i].count;
    return n;
}

template <size_t C>
constexpr size_t gridHeight(const Column (&cols)[C]) {
    size_t h = 0;
    for (size_t i = 0; i < C; ++i) {
        size_t top = size_t(cols[i].count) + cols[i].yOffset;
        if (top > h) h = top;
    }
    return h;
}

template <size_t W, size_t H, size_t N>
struct Layout {
    static constexpr uint8_t width = W;
    static constexpr uint8_t height = H;
    static constexpr uint16_t numLeds = N;

    int16_t grid[W][H]{};  // strip index per cell, or NO_LED
    Point pos[N]{};        // grid position per strip index

    constexpr int16_t index(int x, int y) const {
        if (x < 0 || y < 0 || x >= int(W) || y >= int(H)) return NO_LED;
        return grid[x][y];
    }
};

// Strip index of the first LED of each column.
template <size_t C>
constexpr size_t columnStart(const Column (&cols)[C], size_t column) {
    size_t n = 0;
    for (size_t i = 0; i < column; ++i) n += cols[i].count;
    return n;
}

// Strip index of a LED given by column and row-from-bottom, or NO_LED if out of range.
template <size_t C>
constexpr int16_t columnLedIndex(const Column (&cols)[C], ColumnLed led) {
    if (led.column >= C || led.row >= cols[led.column].count) return NO_LED;
    size_t start = columnStart(cols, led.column);
    bool up = (led.column % 2) == 0;
    return int16_t(start + (up ? led.row : cols[led.column].count - 1 - led.row));
}

template <size_t W, size_t H, size_t N>
constexpr Layout<W, H, N> buildLayout(const Column (&cols)[W]) {
    static_assert(W > 0 && W <= 255, "column count out of range");
    static_assert(H > 0 && H <= 255, "grid height out of range");
    static_assert(N > 0 && N <= 32767, "LED count out of range");
    Layout<W, H, N> l{};
    for (size_t x = 0; x < W; ++x)
        for (size_t y = 0; y < H; ++y) l.grid[x][y] = NO_LED;
    size_t i = 0;
    for (size_t c = 0; c < W; ++c) {
        uint8_t x = uint8_t(W - 1 - c);
        for (size_t k = 0; k < cols[c].count; ++k, ++i) {
            bool up = (c % 2) == 0;
            uint8_t y = uint8_t(cols[c].yOffset + (up ? k : cols[c].count - 1 - k));
            l.grid[x][y] = int16_t(i);
            l.pos[i] = Point{x, y};
        }
    }
    return l;
}

}  // namespace owl

// Declares OWL_LAYOUT from a Column array (use in include/layout.h).
#define OWL_DEFINE_LAYOUT(cols)                                                             \
    constexpr auto OWL_LAYOUT =                                                             \
        ::owl::buildLayout<sizeof(cols) / sizeof(cols[0]), ::owl::gridHeight(cols),         \
                           ::owl::totalLeds(cols)>(cols)
