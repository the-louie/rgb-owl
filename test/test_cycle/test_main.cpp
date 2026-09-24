#include <unity.h>

#include "owl/cycle.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_holds_until_interval(void) {
    Cycler c(3, 1000, 200);
    Cycler::State s = c.update(999);
    TEST_ASSERT_EQUAL(0, s.current);
    TEST_ASSERT_EQUAL(-1, s.next);
    TEST_ASSERT_EQUAL(-1, s.started);
}

static void test_fades_to_next(void) {
    Cycler c(3, 1000, 200);
    Cycler::State s = c.update(1000);
    TEST_ASSERT_EQUAL(0, s.current);
    TEST_ASSERT_EQUAL(1, s.next);
    TEST_ASSERT_EQUAL(1, s.started);
    TEST_ASSERT_EQUAL(0, s.mix);
    s = c.update(100);
    TEST_ASSERT_EQUAL(127, s.mix);
    TEST_ASSERT_EQUAL(-1, s.started);
    s = c.update(100);
    TEST_ASSERT_EQUAL(1, s.current);
    TEST_ASSERT_EQUAL(-1, s.next);
    TEST_ASSERT_EQUAL(0, s.mix);
}

static void test_wraps_around(void) {
    Cycler c(2, 10, 5);
    c.update(10);
    c.update(5);   // now at 1
    c.update(10);  // fade to 0
    Cycler::State s = c.update(5);
    TEST_ASSERT_EQUAL(0, s.current);
}

static void test_zero_fade_is_hard_cut(void) {
    Cycler c(3, 100, 0);
    Cycler::State s = c.update(100);
    TEST_ASSERT_EQUAL(1, s.current);
    TEST_ASSERT_EQUAL(-1, s.next);
    TEST_ASSERT_EQUAL(1, s.started);
}

static void test_auto_off_holds(void) {
    Cycler c(3, 100, 10);
    c.setAuto(false);
    Cycler::State s = c.update(100000);
    TEST_ASSERT_EQUAL(0, s.current);
    TEST_ASSERT_EQUAL(-1, s.next);
}

static void test_select_fades_to_chosen(void) {
    Cycler c(4, 1000, 100);
    c.select(3);
    Cycler::State s = c.update(0);
    TEST_ASSERT_EQUAL(3, s.next);
    s = c.update(100);
    TEST_ASSERT_EQUAL(3, s.current);
}

static void test_select_during_fade_finishes_first(void) {
    Cycler c(4, 100, 100);
    c.update(100);  // fading 0 -> 1
    c.select(2);    // 1 becomes current, fade to 2
    Cycler::State s = c.update(0);
    TEST_ASSERT_EQUAL(1, s.current);
    TEST_ASSERT_EQUAL(2, s.next);
}

static void test_select_current_or_invalid_is_noop(void) {
    Cycler c(2, 1000, 100);
    c.select(0);
    c.select(5);
    Cycler::State s = c.update(0);
    TEST_ASSERT_EQUAL(-1, s.next);
}

static void test_interval_restarts_after_fade(void) {
    Cycler c(3, 100, 10);
    c.update(100);
    c.update(10);                      // now 1, elapsed reset
    Cycler::State s = c.update(99);
    TEST_ASSERT_EQUAL(-1, s.next);
}

static void test_mask_skips_disabled(void) {
    Cycler c(4, 100, 0);
    c.setMask(0b1001);  // only 0 and 3
    Cycler::State s = c.update(100);
    TEST_ASSERT_EQUAL(3, s.current);
    s = c.update(100);
    TEST_ASSERT_EQUAL(0, s.current);
}

static void test_mask_with_only_current_enabled_stays(void) {
    Cycler c(3, 100, 0);
    c.setMask(0b001);
    Cycler::State s = c.update(100);
    TEST_ASSERT_EQUAL(0, s.current);
    TEST_ASSERT_EQUAL(-1, s.started);
}

static void test_mask_does_not_block_manual_select(void) {
    Cycler c(3, 100, 0);
    c.setMask(0b001);
    c.select(2);
    TEST_ASSERT_EQUAL(2, c.update(0).current);
    TEST_ASSERT_EQUAL(0, c.update(100).current);  // auto moves on to the enabled one
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_holds_until_interval);
    RUN_TEST(test_fades_to_next);
    RUN_TEST(test_wraps_around);
    RUN_TEST(test_zero_fade_is_hard_cut);
    RUN_TEST(test_auto_off_holds);
    RUN_TEST(test_select_fades_to_chosen);
    RUN_TEST(test_select_during_fade_finishes_first);
    RUN_TEST(test_select_current_or_invalid_is_noop);
    RUN_TEST(test_interval_restarts_after_fade);
    RUN_TEST(test_mask_skips_disabled);
    RUN_TEST(test_mask_with_only_current_enabled_stays);
    RUN_TEST(test_mask_does_not_block_manual_select);
    return UNITY_END();
}
