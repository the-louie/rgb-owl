#include <unity.h>

#include "layout.h"
#include "owl/layout_map.h"

using namespace owl;

void setUp() {}
void tearDown() {}

// 3 columns right->left: c0 {2 LEDs, y0}, c1 {3, y0}, c2 {1, y2}
//   grid (x=0 left):  y2: [5][2][ ]
//                     y1: [ ][3][1]
//                     y0: [ ][4][0]
constexpr Column SMALL[] = {{2, 0}, {3, 0}, {1, 2}};
OWL_DEFINE_LAYOUT(SMALL);

static void test_dimensions(void) {
    TEST_ASSERT_EQUAL(3, OWL_LAYOUT.width);
    TEST_ASSERT_EQUAL(3, OWL_LAYOUT.height);
    TEST_ASSERT_EQUAL(6, OWL_LAYOUT.numLeds);
}

static void test_zigzag_from_bottom_right(void) {
    TEST_ASSERT_EQUAL(0, OWL_LAYOUT.index(2, 0));
    TEST_ASSERT_EQUAL(1, OWL_LAYOUT.index(2, 1));
    TEST_ASSERT_EQUAL(2, OWL_LAYOUT.index(1, 2));
    TEST_ASSERT_EQUAL(3, OWL_LAYOUT.index(1, 1));
    TEST_ASSERT_EQUAL(4, OWL_LAYOUT.index(1, 0));
    TEST_ASSERT_EQUAL(5, OWL_LAYOUT.index(0, 2));
}

static void test_empty_and_out_of_range(void) {
    TEST_ASSERT_EQUAL(NO_LED, OWL_LAYOUT.index(2, 2));
    TEST_ASSERT_EQUAL(NO_LED, OWL_LAYOUT.index(0, 0));
    TEST_ASSERT_EQUAL(NO_LED, OWL_LAYOUT.index(-1, 0));
    TEST_ASSERT_EQUAL(NO_LED, OWL_LAYOUT.index(3, 0));
    TEST_ASSERT_EQUAL(NO_LED, OWL_LAYOUT.index(0, 3));
}

static void test_pos_is_inverse_of_grid(void) {
    for (int i = 0; i < OWL_LAYOUT.numLeds; ++i) {
        Point p = OWL_LAYOUT.pos[i];
        TEST_ASSERT_EQUAL(i, OWL_LAYOUT.index(p.x, p.y));
    }
}

static void test_column_led_index(void) {
    TEST_ASSERT_EQUAL(1, columnLedIndex(SMALL, {0, 1}));
    TEST_ASSERT_EQUAL(4, columnLedIndex(SMALL, {1, 0}));  // down-running column
    TEST_ASSERT_EQUAL(2, columnLedIndex(SMALL, {1, 2}));
    TEST_ASSERT_EQUAL(5, columnLedIndex(SMALL, {2, 0}));
    TEST_ASSERT_EQUAL(NO_LED, columnLedIndex(SMALL, {1, 3}));
    TEST_ASSERT_EQUAL(NO_LED, columnLedIndex(SMALL, {3, 0}));
}

static void test_config_layout_is_consistent(void) {
    using namespace owl::config;
    TEST_ASSERT_EQUAL(totalLeds(COLUMNS), config::OWL_LAYOUT.numLeds);
    int mapped = 0;
    for (int x = 0; x < config::OWL_LAYOUT.width; ++x)
        for (int y = 0; y < config::OWL_LAYOUT.height; ++y)
            if (config::OWL_LAYOUT.index(x, y) != NO_LED) ++mapped;
    TEST_ASSERT_EQUAL(config::OWL_LAYOUT.numLeds, mapped);
    for (const auto& e : EYES) TEST_ASSERT_NOT_EQUAL(NO_LED, columnLedIndex(COLUMNS, e));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_dimensions);
    RUN_TEST(test_zigzag_from_bottom_right);
    RUN_TEST(test_empty_and_out_of_range);
    RUN_TEST(test_pos_is_inverse_of_grid);
    RUN_TEST(test_column_led_index);
    RUN_TEST(test_config_layout_is_consistent);
    return UNITY_END();
}
