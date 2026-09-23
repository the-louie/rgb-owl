#include <unity.h>

#include "owl/settings.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_defaults_match_spec(void) {
    Settings s;
    TEST_ASSERT_TRUE(s.on);
    TEST_ASSERT_TRUE(s.autoCycle);
    TEST_ASSERT_EQUAL(60, s.intervalS);
    TEST_ASSERT_EQUAL(2000, s.fadeMs);
}

static void test_apply_numbers(void) {
    Settings s;
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "brightness", "200", 7));
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "speed", "0", 7));
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "hue", "42", 7));
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "interval", "90", 7));
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "fade", "0", 7));
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "effect", "6", 7));
    TEST_ASSERT_EQUAL(200, s.brightness);
    TEST_ASSERT_EQUAL(0, s.speed);
    TEST_ASSERT_EQUAL(42, s.hue);
    TEST_ASSERT_EQUAL(90, s.intervalS);
    TEST_ASSERT_EQUAL(0, s.fadeMs);
    TEST_ASSERT_EQUAL(6, s.effect);
}

static void test_apply_clamps(void) {
    Settings s;
    apply(s, "brightness", "0", 7);
    TEST_ASSERT_EQUAL(1, s.brightness);
    apply(s, "brightness", "999", 7);
    TEST_ASSERT_EQUAL(255, s.brightness);
    apply(s, "interval", "1", 7);
    TEST_ASSERT_EQUAL(5, s.intervalS);
    apply(s, "fade", "99999", 7);
    TEST_ASSERT_EQUAL(10000, s.fadeMs);
}

static void test_apply_bools(void) {
    Settings s;
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "on", "off", 7));
    TEST_ASSERT_FALSE(s.on);
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "auto", "0", 7));
    TEST_ASSERT_FALSE(s.autoCycle);
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "on", "true", 7));
    TEST_ASSERT_TRUE(s.on);
    TEST_ASSERT_EQUAL(ApplyResult::BadValue, apply(s, "on", "maybe", 7));
}

static void test_apply_rejects(void) {
    Settings s;
    Settings before = s;
    TEST_ASSERT_EQUAL(ApplyResult::BadValue, apply(s, "brightness", "abc", 7));
    TEST_ASSERT_EQUAL(ApplyResult::BadValue, apply(s, "brightness", "", 7));
    TEST_ASSERT_EQUAL(ApplyResult::BadValue, apply(s, "brightness", "-5", 7));
    TEST_ASSERT_EQUAL(ApplyResult::BadValue, apply(s, "effect", "7", 7));
    TEST_ASSERT_EQUAL(ApplyResult::UnknownKey, apply(s, "colour", "1", 7));
    TEST_ASSERT_TRUE(s == before);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_defaults_match_spec);
    RUN_TEST(test_apply_numbers);
    RUN_TEST(test_apply_clamps);
    RUN_TEST(test_apply_bools);
    RUN_TEST(test_apply_rejects);
    return UNITY_END();
}
