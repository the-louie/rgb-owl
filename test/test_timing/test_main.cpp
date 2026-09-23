#include <unity.h>

#include "owl/timing.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_pacer_fires_once_per_period(void) {
    FramePacer p(10);
    TEST_ASSERT_TRUE(p.due(100));
    TEST_ASSERT_FALSE(p.due(105));
    TEST_ASSERT_TRUE(p.due(110));
    TEST_ASSERT_FALSE(p.due(119));
    TEST_ASSERT_TRUE(p.due(120));
}

static void test_pacer_skips_missed_frames(void) {
    FramePacer p(10);
    p.due(0);
    TEST_ASSERT_TRUE(p.due(55));   // late by several frames: one frame only
    TEST_ASSERT_FALSE(p.due(60));
    TEST_ASSERT_TRUE(p.due(65));
}

static void test_pacer_millis_wrap(void) {
    FramePacer p(10);
    p.due(0xFFFFFFF8u);
    TEST_ASSERT_FALSE(p.due(0xFFFFFFFEu));
    TEST_ASSERT_TRUE(p.due(2));
}

static void test_walk_index(void) {
    TEST_ASSERT_EQUAL(0, walkIndex(0, 40, 3));
    TEST_ASSERT_EQUAL(1, walkIndex(40, 40, 3));
    TEST_ASSERT_EQUAL(2, walkIndex(119, 40, 3));
    TEST_ASSERT_EQUAL(-1, walkIndex(120, 40, 3));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_pacer_fires_once_per_period);
    RUN_TEST(test_pacer_skips_missed_frames);
    RUN_TEST(test_pacer_millis_wrap);
    RUN_TEST(test_walk_index);
    return UNITY_END();
}
