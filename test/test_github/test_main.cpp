#include <unity.h>

#include "owl/github.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static const char* norm(const char* in) {
    static char out[80];
    return parseGithubProject(in, out, sizeof(out)) ? out : nullptr;
}

static void test_accepts_forms(void) {
    TEST_ASSERT_EQUAL_STRING("louie/bodforss-rgb-owl", norm("https://github.com/louie/bodforss-rgb-owl"));
    TEST_ASSERT_EQUAL_STRING("louie/owl", norm("https://github.com/louie/owl.git"));
    TEST_ASSERT_EQUAL_STRING("louie/owl", norm("https://github.com/louie/owl/releases/latest"));
    TEST_ASSERT_EQUAL_STRING("louie/owl", norm("github.com/louie/owl"));
    TEST_ASSERT_EQUAL_STRING("louie/owl", norm("louie/owl"));
    TEST_ASSERT_EQUAL_STRING("a-b/c.d_e", norm("a-b/c.d_e"));
}

static void test_rejects(void) {
    TEST_ASSERT_NULL(norm(""));
    TEST_ASSERT_NULL(norm("owl"));
    TEST_ASSERT_NULL(norm("/owl"));
    TEST_ASSERT_NULL(norm("louie/"));
    TEST_ASSERT_NULL(norm("https://gitlab.com/louie/owl"));
    TEST_ASSERT_NULL(norm("louie/o wl"));
    TEST_ASSERT_NULL(norm("louie/\"owl"));
    TEST_ASSERT_NULL(norm("gitlab.com/x/y"));  // dot in owner: another host, not a GitHub owner
    TEST_ASSERT_NULL(norm("a_b/c"));
}

static void test_too_long(void) {
    char out[8];
    TEST_ASSERT_FALSE(parseGithubProject("owner/repository", out, sizeof(out)));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_accepts_forms);
    RUN_TEST(test_rejects);
    RUN_TEST(test_too_long);
    return UNITY_END();
}
