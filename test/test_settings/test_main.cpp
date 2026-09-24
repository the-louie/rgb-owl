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

static void test_to_json(void) {
    Settings s;
    s.on = false;
    s.effect = 3;
    char buf[200];
    size_t n = toJson(buf, sizeof(buf), s, 5);
    TEST_ASSERT_EQUAL_STRING(
        "{\"on\":false,\"effect\":3,\"current\":5,\"auto\":true,\"interval\":60,"
        "\"fade\":2000,\"brightness\":128,\"speed\":128,\"hue\":160}",
        buf);
    TEST_ASSERT_EQUAL(strlen(buf), n);
}

static void test_to_json_too_small(void) {
    char buf[10];
    TEST_ASSERT_EQUAL(0, toJson(buf, sizeof(buf), Settings{}, 0));
}

static void test_cycle_mask(void) {
    Settings s;
    TEST_ASSERT_EQUAL_HEX32(0x7FFFFFFF, s.cycle);
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "cycle", "5", 7));
    TEST_ASSERT_EQUAL(5, s.cycle);
    TEST_ASSERT_EQUAL(ApplyResult::BadValue, apply(s, "cycle", "x", 7));
}

static void test_night_fields(void) {
    Settings s;
    TEST_ASSERT_TRUE(s.night);
    TEST_ASSERT_EQUAL(1380, s.nightFrom);
    TEST_ASSERT_EQUAL(420, s.nightTo);
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "night", "0", 7));
    TEST_ASSERT_FALSE(s.night);
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "night_from", "1320", 7));
    TEST_ASSERT_EQUAL(ApplyResult::Ok, apply(s, "night_to", "9999", 7));
    TEST_ASSERT_EQUAL(1320, s.nightFrom);
    TEST_ASSERT_EQUAL(1439, s.nightTo);
}

static void test_is_night(void) {
    TEST_ASSERT_TRUE(isNight(23 * 60, 1380, 420));   // 23:00
    TEST_ASSERT_TRUE(isNight(3 * 60, 1380, 420));    // 03:00
    TEST_ASSERT_FALSE(isNight(7 * 60, 1380, 420));   // 07:00 = end, exclusive
    TEST_ASSERT_FALSE(isNight(12 * 60, 1380, 420));
    TEST_ASSERT_TRUE(isNight(13 * 60, 12 * 60, 14 * 60));  // same-day window
    TEST_ASSERT_FALSE(isNight(15 * 60, 12 * 60, 14 * 60));
    TEST_ASSERT_FALSE(isNight(100, 500, 500));             // empty window
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_defaults_match_spec);
    RUN_TEST(test_apply_numbers);
    RUN_TEST(test_apply_clamps);
    RUN_TEST(test_apply_bools);
    RUN_TEST(test_apply_rejects);
    RUN_TEST(test_to_json);
    RUN_TEST(test_to_json_too_small);
    RUN_TEST(test_cycle_mask);
    RUN_TEST(test_night_fields);
    RUN_TEST(test_is_night);
    return UNITY_END();
}
