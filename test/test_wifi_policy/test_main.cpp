#include <unity.h>

#include "owl/wifi_policy.h"

using namespace owl;
using A = WifiPolicy::Action;
using L = WifiPolicy::Link;
using S = WifiPolicy::Status;

void setUp() {}
void tearDown() {}

static const uint32_t CONNECT = 15000, WINDOW = 180000;

static void test_unconfigured_stays_off(void) {
    WifiPolicy p(CONNECT, WINDOW);
    TEST_ASSERT_TRUE(p.begin(0, false) == A::None);
    TEST_ASSERT_TRUE(p.status() == S::Unconfigured);
    TEST_ASSERT_TRUE(p.update(1000, false) == A::None);
    TEST_ASSERT_TRUE(p.link() == L::Off);
}

static void test_boot_window_then_off(void) {
    WifiPolicy p(CONNECT, WINDOW);
    TEST_ASSERT_TRUE(p.begin(0, true) == A::Start);
    TEST_ASSERT_TRUE(p.update(3000, true) == A::None);
    TEST_ASSERT_TRUE(p.status() == S::Connected);
    TEST_ASSERT_TRUE(p.windowOpen(3000));
    TEST_ASSERT_TRUE(p.update(3000 + WINDOW - 1, true) == A::None);
    TEST_ASSERT_TRUE(p.update(3000 + WINDOW, true) == A::Stop);
    TEST_ASSERT_TRUE(p.status() == S::Off);
    TEST_ASSERT_TRUE(p.update(999999, false) == A::None);  // stays off
}

static void test_connect_failure(void) {
    WifiPolicy p(CONNECT, WINDOW);
    p.begin(0, true);
    TEST_ASSERT_TRUE(p.update(CONNECT - 1, false) == A::None);
    TEST_ASSERT_TRUE(p.update(CONNECT, false) == A::Stop);
    TEST_ASSERT_TRUE(p.status() == S::Failed);
    TEST_ASSERT_FALSE(p.windowOpen(CONNECT));
}

static void test_devmode_keeps_wifi_on(void) {
    WifiPolicy p(CONNECT, WINDOW);
    p.begin(0, true);
    p.update(1000, true);
    p.setDevmode(true);
    TEST_ASSERT_TRUE(p.update(1000 + WINDOW * 5, true) == A::None);
    TEST_ASSERT_TRUE(p.link() == L::Online);
    p.setDevmode(false);
    TEST_ASSERT_TRUE(p.update(1000 + WINDOW * 5 + 1, true) == A::Stop);
}

static void test_devmode_turns_wifi_on_when_off(void) {
    WifiPolicy p(CONNECT, WINDOW);
    p.begin(0, true);
    p.update(1000, true);
    p.update(1000 + WINDOW, true);  // off
    p.setDevmode(true);
    TEST_ASSERT_TRUE(p.update(500000, false) == A::Start);
    TEST_ASSERT_TRUE(p.update(501000, true) == A::None);
    TEST_ASSERT_TRUE(p.link() == L::Online);
}

static void test_devmode_keeps_retrying(void) {
    WifiPolicy p(CONNECT, WINDOW);
    p.begin(0, true);
    p.setDevmode(true);
    TEST_ASSERT_TRUE(p.update(CONNECT, false) == A::None);
    TEST_ASSERT_TRUE(p.status() == S::Failed);
    TEST_ASSERT_TRUE(p.link() == L::Connecting);
}

static void test_link_loss_reconnects_within_window(void) {
    WifiPolicy p(CONNECT, WINDOW);
    p.begin(0, true);
    p.update(1000, true);
    TEST_ASSERT_TRUE(p.update(5000, false) == A::None);
    TEST_ASSERT_TRUE(p.status() == S::Connecting);
    TEST_ASSERT_TRUE(p.update(6000, true) == A::None);
    TEST_ASSERT_TRUE(p.status() == S::Connected);
    TEST_ASSERT_TRUE(p.update(1000 + WINDOW, true) == A::Stop);  // window not restarted
}

static void test_devmode_needs_configuration(void) {
    WifiPolicy p(CONNECT, WINDOW);
    p.begin(0, false);
    p.setDevmode(true);
    TEST_ASSERT_TRUE(p.update(1000, false) == A::None);
    p.setConfigured(true);
    TEST_ASSERT_TRUE(p.update(2000, false) == A::Start);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_unconfigured_stays_off);
    RUN_TEST(test_boot_window_then_off);
    RUN_TEST(test_connect_failure);
    RUN_TEST(test_devmode_keeps_wifi_on);
    RUN_TEST(test_devmode_turns_wifi_on_when_off);
    RUN_TEST(test_devmode_keeps_retrying);
    RUN_TEST(test_link_loss_reconnects_within_window);
    RUN_TEST(test_devmode_needs_configuration);
    return UNITY_END();
}
