#include <unity.h>

#include "owl/protocol.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_verb_only(void) {
    Command c;
    TEST_ASSERT_TRUE(c.parse("wifi_scan"));
    TEST_ASSERT_EQUAL_STRING("wifi_scan", c.verb());
    TEST_ASSERT_EQUAL(0, c.size());
}

static void test_pairs_and_decoding(void) {
    Command c;
    TEST_ASSERT_TRUE(c.parse("wifi_test ssid=My+Net%21&pass=a%26b%3Dc"));
    TEST_ASSERT_EQUAL_STRING("wifi_test", c.verb());
    TEST_ASSERT_EQUAL(2, c.size());
    TEST_ASSERT_EQUAL_STRING("My Net!", c.get("ssid"));
    TEST_ASSERT_EQUAL_STRING("a&b=c", c.get("pass"));
    TEST_ASSERT_NULL(c.get("nope"));
}

static void test_empty_value_allowed(void) {
    Command c;
    TEST_ASSERT_TRUE(c.parse("wifi_save ssid=Open&pass="));
    TEST_ASSERT_EQUAL_STRING("", c.get("pass"));
}

static void test_rejects_malformed(void) {
    Command c;
    TEST_ASSERT_FALSE(c.parse(""));
    TEST_ASSERT_FALSE(c.parse("Set x=1"));       // verb must be lowercase
    TEST_ASSERT_FALSE(c.parse("set x"));         // missing '='
    TEST_ASSERT_FALSE(c.parse("set =1"));        // empty key
    TEST_ASSERT_FALSE(c.parse("set x=%zz"));     // bad escape
    TEST_ASSERT_FALSE(c.parse("set x=%00"));     // NUL
    TEST_ASSERT_FALSE(c.parse("set x=%4"));      // truncated escape
    char big[600];
    memset(big, 'a', sizeof(big) - 1);
    big[sizeof(big) - 1] = 0;
    TEST_ASSERT_FALSE(c.parse(big));
    TEST_ASSERT_EQUAL(0, c.size());
}

static void test_too_many_pairs(void) {
    Command c;
    TEST_ASSERT_FALSE(c.parse("set a=1&b=1&c=1&d=1&e=1&f=1&g=1&h=1&i=1&j=1&k=1&l=1&m=1"));
    TEST_ASSERT_TRUE(c.parse("set a=1&b=1&c=1&d=1&e=1&f=1&g=1&h=1&i=1&j=1&k=1&l=1"));
}

static void test_json_writer(void) {
    char buf[128];
    JsonWriter w(buf, sizeof(buf));
    w.str("type", "wifi_scan").num("rssi", -52).boolean("ok", true).str("ssid", "a\"b\\c\n").raw("list", "[1,2]");
    TEST_ASSERT_EQUAL_STRING(
        "{\"type\":\"wifi_scan\",\"rssi\":-52,\"ok\":true,\"ssid\":\"a\\\"b\\\\c\\u000a\",\"list\":[1,2]}",
        w.finish());
}

static void test_json_writer_overflow(void) {
    char buf[16];
    JsonWriter w(buf, sizeof(buf));
    w.str("key", "a long value that does not fit");
    TEST_ASSERT_NULL(w.finish());
}

static int cmp(const char* a, const char* b) {
    SemVer x, y;
    TEST_ASSERT_TRUE_MESSAGE(x.parse(a), a);
    TEST_ASSERT_TRUE_MESSAGE(y.parse(b), b);
    return compare(x, y);
}

static void test_semver_order(void) {
    TEST_ASSERT_TRUE(cmp("1.0.0", "1.0.1") < 0);
    TEST_ASSERT_TRUE(cmp("v1.2.0", "1.10.0") < 0);
    TEST_ASSERT_TRUE(cmp("2.0.0", "1.99.99") > 0);
    TEST_ASSERT_EQUAL(0, cmp("1.2.3", "v1.2.3+abc"));
    TEST_ASSERT_TRUE(cmp("1.0.0-rc.1", "1.0.0") < 0);
    TEST_ASSERT_TRUE(cmp("1.0.0-alpha", "1.0.0-alpha.1") < 0);
    TEST_ASSERT_TRUE(cmp("1.0.0-alpha.1", "1.0.0-alpha.beta") < 0);
    TEST_ASSERT_TRUE(cmp("1.0.0-beta.2", "1.0.0-beta.11") < 0);
    TEST_ASSERT_TRUE(cmp("1.0.1-dev.3+abc", "1.0.0") > 0);   // dev build after v1.0.0 (tools/version.py)
    TEST_ASSERT_TRUE(cmp("1.0.1-dev.3+abc", "1.0.1") < 0);   // ...but before the next release
    TEST_ASSERT_TRUE(cmp("0.0.0-dev+abc-dirty", "0.1.0") < 0);
}

static void test_semver_rejects(void) {
    SemVer v;
    TEST_ASSERT_FALSE(v.parse("1.0"));
    TEST_ASSERT_FALSE(v.parse("x1.0.0"));
    TEST_ASSERT_FALSE(v.parse("1.0.0-"));
    TEST_ASSERT_FALSE(v.parse("1.0.0 "));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_verb_only);
    RUN_TEST(test_pairs_and_decoding);
    RUN_TEST(test_empty_value_allowed);
    RUN_TEST(test_rejects_malformed);
    RUN_TEST(test_too_many_pairs);
    RUN_TEST(test_json_writer);
    RUN_TEST(test_json_writer_overflow);
    RUN_TEST(test_semver_order);
    RUN_TEST(test_semver_rejects);
    return UNITY_END();
}
