#include <unity.h>

#include "owl/wifi_fsm.h"

using namespace owl;
using S = WifiFsm::State;
using A = WifiFsm::Action;

void setUp() {}
void tearDown() {}

static void test_connects(void) {
    WifiFsm f(15000, 300000);
    f.begin(0);
    TEST_ASSERT_TRUE(f.update(1000, false) == A::None);
    TEST_ASSERT_TRUE(f.update(2000, true) == A::WentOnline);
    TEST_ASSERT_TRUE(f.state() == S::Online);
}

static void test_portal_after_timeout(void) {
    WifiFsm f(15000, 300000);
    f.begin(0);
    TEST_ASSERT_TRUE(f.update(14999, false) == A::None);
    TEST_ASSERT_TRUE(f.update(15000, false) == A::StartPortal);
    TEST_ASSERT_TRUE(f.state() == S::Portal);
}

static void test_portal_configured(void) {
    WifiFsm f(15000, 300000);
    f.begin(0);
    f.update(15000, false);
    TEST_ASSERT_TRUE(f.update(60000, true) == A::StopPortal);
    TEST_ASSERT_TRUE(f.state() == S::Online);
}

static void test_portal_times_out_and_retries(void) {
    WifiFsm f(15000, 300000);
    f.begin(0);
    f.update(15000, false);
    TEST_ASSERT_TRUE(f.update(315000, false) == A::Retry);
    TEST_ASSERT_TRUE(f.state() == S::Connecting);
    TEST_ASSERT_TRUE(f.update(330000, false) == A::StartPortal);
}

static void test_link_lost_reconnects(void) {
    WifiFsm f(15000, 300000);
    f.begin(0);
    f.update(1000, true);
    TEST_ASSERT_TRUE(f.update(5000, false) == A::None);
    TEST_ASSERT_TRUE(f.state() == S::Connecting);
    TEST_ASSERT_TRUE(f.update(20000, false) == A::StartPortal);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_connects);
    RUN_TEST(test_portal_after_timeout);
    RUN_TEST(test_portal_configured);
    RUN_TEST(test_portal_times_out_and_retries);
    RUN_TEST(test_link_lost_reconnects);
    return UNITY_END();
}
