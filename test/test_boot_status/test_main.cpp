#include <unity.h>

#include "owl/boot_status.h"

using namespace owl;
using W = WifiPolicy::Status;
using U = UpdateStatus;
using B = BootStatus;

void setUp() {}
void tearDown() {}

static bool is(const Visual& a, const Visual& b) { return a == b; }

static void test_ble_phase_colour(void) {
    B s;
    s.begin(0, true);
    TEST_ASSERT_TRUE(is(s.update(10, W::Connecting, U::None, 0), B::GREEN));
    s.begin(0, false);
    TEST_ASSERT_TRUE(is(s.update(10, W::Connecting, U::None, 0), B::BLUE));
}

static void test_unconfigured_red_then_done(void) {
    B s;
    s.begin(0, true);
    TEST_ASSERT_TRUE(is(s.update(B::FLASH_MS, W::Unconfigured, U::None, 0), B::RED));
    TEST_ASSERT_FALSE(s.done());
    s.update(B::FLASH_MS + B::RESULT_MS + 1, W::Unconfigured, U::None, 0);
    TEST_ASSERT_TRUE(s.done());
}

static void test_connecting_then_failed_blinks(void) {
    B s;
    s.begin(0, true);
    TEST_ASSERT_TRUE(is(s.update(2000, W::Connecting, U::None, 0), B::YELLOW_PULSE));
    TEST_ASSERT_TRUE(is(s.update(15000, W::Failed, U::None, 0), B::RED_BLINK));
    s.update(15000 + B::RESULT_MS + 1, W::Off, U::None, 0);
    TEST_ASSERT_TRUE(s.done());
}

static void test_connected_green_then_update_check(void) {
    B s;
    s.begin(0, true);
    s.update(1500, W::Connecting, U::None, 0);
    TEST_ASSERT_TRUE(is(s.update(3000, W::Connected, U::Checking, 0), B::GREEN));  // connected flash first
    TEST_ASSERT_TRUE(is(s.update(3000 + B::FLASH_MS + 1, W::Connected, U::Checking, 0), B::PURPLE_PULSE));
    TEST_ASSERT_TRUE(is(s.update(6000, W::Connected, U::UpToDate, 0), B::GREEN));
    TEST_ASSERT_FALSE(s.done());
    s.update(6000 + B::RESULT_MS + 1, W::Connected, U::UpToDate, 0);
    TEST_ASSERT_TRUE(s.done());
}

static void test_download_fills_and_never_finishes(void) {
    B s;
    s.begin(0, true);
    s.update(1500, W::Connected, U::Checking, 0);
    Visual v = s.update(5000, W::Connected, U::Downloading, 42);
    TEST_ASSERT_EQUAL(Visual::Fill, v.kind);
    TEST_ASSERT_EQUAL(42, v.fill);
    s.update(900000, W::Connected, U::Downloading, 150);
    TEST_ASSERT_FALSE(s.done());
    TEST_ASSERT_EQUAL(100, s.update(900001, W::Connected, U::Downloading, 150).fill);
}

static void test_no_update_check_finishes_after_result(void) {
    B s;
    s.begin(0, false);
    s.update(1500, W::Connected, U::None, 0);
    s.update(1500 + B::FLASH_MS + 1, W::Connected, U::None, 0);
    TEST_ASSERT_FALSE(s.done());
    s.update(1500 + B::FLASH_MS + B::RESULT_MS + 2, W::Connected, U::None, 0);
    TEST_ASSERT_TRUE(s.done());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_ble_phase_colour);
    RUN_TEST(test_unconfigured_red_then_done);
    RUN_TEST(test_connecting_then_failed_blinks);
    RUN_TEST(test_connected_green_then_update_check);
    RUN_TEST(test_download_fills_and_never_finishes);
    RUN_TEST(test_no_update_check_finishes_after_result);
    return UNITY_END();
}
