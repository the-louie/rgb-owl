#include <unity.h>

#include "owl/blink.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_open_during_gap(void) {
    Blink b(1000);
    TEST_ASSERT_EQUAL(255, b.update(0, 500));
    TEST_ASSERT_EQUAL(255, b.update(999, 500));
}

static void test_blink_shape(void) {
    Blink b(1000);
    TEST_ASSERT_EQUAL(255 - 40 * 255 / 80, b.update(1040, 500));  // half closed
    TEST_ASSERT_EQUAL(0, b.update(40, 500));                        // shut at 1080
    TEST_ASSERT_EQUAL(0, b.update(99, 500));                        // still shut at 1179
    TEST_ASSERT_EQUAL(60 * 255 / 120, b.update(61, 500));           // opening at 1240
}

static void test_uses_next_gap_after_blink(void) {
    Blink b(1000);
    b.update(1000 + Blink::BLINK_MS, 500);                // blink done, new gap 500
    TEST_ASSERT_EQUAL(255, b.update(499, 2000));
    TEST_ASSERT_EQUAL(255 - 10 * 255 / 80, b.update(11, 2000));
}

static void test_large_dt_skips_whole_cycles(void) {
    Blink b(100);
    TEST_ASSERT_EQUAL(255, b.update(10 * (100 + Blink::BLINK_MS) + 50, 100));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_open_during_gap);
    RUN_TEST(test_blink_shape);
    RUN_TEST(test_uses_next_gap_after_blink);
    RUN_TEST(test_large_dt_skips_whole_cycles);
    return UNITY_END();
}
