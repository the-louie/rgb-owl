#include <unity.h>

#include "owl/debounce.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_nothing_pending(void) {
    SaveDebouncer d(5000);
    TEST_ASSERT_FALSE(d.due(100000));
}

static void test_fires_once_after_delay(void) {
    SaveDebouncer d(5000);
    d.changed(1000);
    TEST_ASSERT_FALSE(d.due(5999));
    TEST_ASSERT_TRUE(d.due(6000));
    TEST_ASSERT_FALSE(d.due(7000));
}

static void test_changes_restart_delay(void) {
    SaveDebouncer d(5000);
    d.changed(1000);
    d.changed(4000);
    TEST_ASSERT_FALSE(d.due(6000));
    TEST_ASSERT_TRUE(d.due(9000));
}

static void test_millis_wrap(void) {
    SaveDebouncer d(5000);
    d.changed(0xFFFFF000u);
    TEST_ASSERT_FALSE(d.due(0x00000100u));
    TEST_ASSERT_TRUE(d.due(0x00000400u));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_nothing_pending);
    RUN_TEST(test_fires_once_after_delay);
    RUN_TEST(test_changes_restart_delay);
    RUN_TEST(test_millis_wrap);
    return UNITY_END();
}
