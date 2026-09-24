#include <unity.h>

#include "owl/hold.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_fires_once_after_hold(void) {
    HoldDetector h(5000);
    TEST_ASSERT_FALSE(h.update(true, 1000));
    TEST_ASSERT_FALSE(h.update(true, 5999));
    TEST_ASSERT_TRUE(h.update(true, 6000));
    TEST_ASSERT_FALSE(h.update(true, 9000));
}

static void test_short_press_does_nothing(void) {
    HoldDetector h(5000);
    h.update(true, 0);
    TEST_ASSERT_FALSE(h.update(false, 4000));
    TEST_ASSERT_FALSE(h.update(true, 4500));  // new press starts over
    TEST_ASSERT_FALSE(h.update(true, 9000));
    TEST_ASSERT_TRUE(h.update(true, 9500));
}

static void test_rearms_after_release(void) {
    HoldDetector h(100);
    h.update(true, 0);
    TEST_ASSERT_TRUE(h.update(true, 100));
    h.update(false, 200);
    h.update(true, 300);
    TEST_ASSERT_TRUE(h.update(true, 400));
}

static void test_held_ms(void) {
    HoldDetector h(5000);
    TEST_ASSERT_EQUAL(0, h.heldMs(10));
    h.update(true, 100);
    TEST_ASSERT_EQUAL(250, h.heldMs(350));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_fires_once_after_hold);
    RUN_TEST(test_short_press_does_nothing);
    RUN_TEST(test_rearms_after_release);
    RUN_TEST(test_held_ms);
    return UNITY_END();
}
