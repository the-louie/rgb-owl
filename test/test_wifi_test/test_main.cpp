#include <unity.h>

#include "owl/wifi_test.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_save_needs_passing_test(void) {
    TestGate g;
    TEST_ASSERT_FALSE(g.allowSave("net", "pw"));
    g.record("net", "pw", false);
    TEST_ASSERT_FALSE(g.allowSave("net", "pw"));
    g.record("net", "pw", true);
    TEST_ASSERT_TRUE(g.allowSave("net", "pw"));
}

static void test_save_needs_same_credentials(void) {
    TestGate g;
    g.record("net", "pw", true);
    TEST_ASSERT_FALSE(g.allowSave("net", "pw2"));
    TEST_ASSERT_FALSE(g.allowSave("net2", "pw"));
    TEST_ASSERT_FALSE(g.allowSave("ne", "tpw"));  // boundary between ssid and password matters
}

static void test_clear(void) {
    TestGate g;
    g.record("net", "pw", true);
    g.clear();
    TEST_ASSERT_FALSE(g.allowSave("net", "pw"));
}

static void test_reasons(void) {
    TEST_ASSERT_EQUAL_STRING("network not found", wifiFailReason(201));
    TEST_ASSERT_EQUAL_STRING("wrong password", wifiFailReason(15));
    TEST_ASSERT_EQUAL_STRING("wrong password", wifiFailReason(202));
    TEST_ASSERT_EQUAL_STRING("timeout", wifiFailReason(0));
    TEST_ASSERT_EQUAL_STRING("connection failed", wifiFailReason(8));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_save_needs_passing_test);
    RUN_TEST(test_save_needs_same_credentials);
    RUN_TEST(test_clear);
    RUN_TEST(test_reasons);
    return UNITY_END();
}
