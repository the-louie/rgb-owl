#include <unity.h>

#include "owl/registry.h"

using namespace owl;

void setUp() {}
void tearDown() {}

struct Named {
    const char* n;
    const char* name() const { return n; }
};

static Named a{"plasma"}, b{"rain"}, c{"flame"};
static Named* const ITEMS[] = {&a, &b, &c};

static void test_size_and_index(void) {
    Registry<Named> r(ITEMS);
    TEST_ASSERT_EQUAL(3, r.size());
    TEST_ASSERT_EQUAL_STRING("rain", r[1].name());
}

static void test_find(void) {
    Registry<Named> r(ITEMS);
    TEST_ASSERT_EQUAL(2, r.find("flame"));
    TEST_ASSERT_EQUAL(-1, r.find("nope"));
}

static void test_next_wraps(void) {
    Registry<Named> r(ITEMS);
    TEST_ASSERT_EQUAL(1, r.next(0));
    TEST_ASSERT_EQUAL(0, r.next(2));
}

static void test_clock_speed_scaling(void) {
    EffectClock c;
    TEST_ASSERT_EQUAL(16, c.advance(16, 128));   // 1x
    TEST_ASSERT_EQUAL(16 + 31, c.advance(16, 255));  // 16*255/128 = 31.875
    EffectClock paused;
    TEST_ASSERT_EQUAL(0, paused.advance(1000, 0));
}

static void test_clock_keeps_fraction(void) {
    EffectClock c;
    for (int i = 0; i < 128; ++i) c.advance(1, 1);  // 1/128 ms per step
    TEST_ASSERT_EQUAL(1, c.now());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_size_and_index);
    RUN_TEST(test_find);
    RUN_TEST(test_next_wraps);
    RUN_TEST(test_clock_speed_scaling);
    RUN_TEST(test_clock_keeps_fraction);
    return UNITY_END();
}
