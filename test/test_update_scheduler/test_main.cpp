#include <unity.h>

#include "owl/update_scheduler.h"

using namespace owl;
using A = UpdateScheduler::Action;
using O = UpdateScheduler::Outcome;

void setUp() {}
void tearDown() {}

static const uint32_t DAY = 86400000, TIMEOUT = 60000;

static void test_boot_check_up_to_date(void) {
    UpdateScheduler s(DAY, TIMEOUT);
    s.begin(0);
    TEST_ASSERT_TRUE(s.tick(0, true, false, O::Pending, false) == A::HoldWifi);
    TEST_ASSERT_TRUE(s.status() == UpdateStatus::Checking);
    TEST_ASSERT_TRUE(s.tick(2000, true, true, O::Pending, false) == A::StartCheck);
    TEST_ASSERT_TRUE(s.tick(3000, true, true, O::Pending, false) == A::None);
    TEST_ASSERT_TRUE(s.tick(4000, true, true, O::UpToDate, false) == A::ReleaseWifi);
    TEST_ASSERT_TRUE(s.status() == UpdateStatus::UpToDate);
    TEST_ASSERT_FALSE(s.busy());
}

static void test_newer_installs(void) {
    UpdateScheduler s(DAY, TIMEOUT);
    s.begin(0);
    s.tick(0, true, true, O::Pending, false);
    s.tick(1, true, true, O::Pending, false);
    TEST_ASSERT_TRUE(s.tick(2, true, true, O::Newer, false) == A::StartInstall);
    TEST_ASSERT_TRUE(s.status() == UpdateStatus::Downloading);
    TEST_ASSERT_TRUE(s.tick(3, true, true, O::Newer, true) == A::ReleaseWifi);  // install failed
    TEST_ASSERT_TRUE(s.status() == UpdateStatus::Failed);
}

static void test_daily_after_boot(void) {
    UpdateScheduler s(DAY, TIMEOUT);
    s.begin(0);
    s.tick(0, true, true, O::Pending, false);
    s.tick(1, true, true, O::Pending, false);
    s.tick(2, true, true, O::UpToDate, false);
    TEST_ASSERT_TRUE(s.tick(DAY - 1, true, false, O::Pending, false) == A::None);
    TEST_ASSERT_TRUE(s.tick(DAY, true, false, O::Pending, false) == A::HoldWifi);
}

static void test_not_enabled_skips(void) {
    UpdateScheduler s(DAY, TIMEOUT);
    s.begin(0);
    TEST_ASSERT_TRUE(s.tick(0, false, true, O::Pending, false) == A::None);
    TEST_ASSERT_TRUE(s.status() == UpdateStatus::None);
    TEST_ASSERT_TRUE(s.tick(1000, true, true, O::Pending, false) == A::None);  // waits for next day
}

static void test_offline_times_out(void) {
    UpdateScheduler s(DAY, TIMEOUT);
    s.begin(0);
    s.tick(0, true, false, O::Pending, false);
    TEST_ASSERT_TRUE(s.tick(TIMEOUT - 1, true, false, O::Pending, false) == A::None);
    TEST_ASSERT_TRUE(s.tick(TIMEOUT, true, false, O::Pending, false) == A::ReleaseWifi);
    TEST_ASSERT_TRUE(s.status() == UpdateStatus::Failed);
}

static void test_check_now(void) {
    UpdateScheduler s(DAY, TIMEOUT);
    s.begin(0);
    s.tick(0, false, true, O::Pending, false);  // skipped at boot (no project yet)
    s.checkNow(5000);
    TEST_ASSERT_TRUE(s.tick(5000, true, true, O::Pending, false) == A::HoldWifi);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_boot_check_up_to_date);
    RUN_TEST(test_newer_installs);
    RUN_TEST(test_daily_after_boot);
    RUN_TEST(test_not_enabled_skips);
    RUN_TEST(test_offline_times_out);
    RUN_TEST(test_check_now);
    return UNITY_END();
}
