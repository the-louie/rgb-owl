#include <unity.h>

#include "owl/gamma.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_identity(void) {
    GammaLut g(1.0f);
    for (int i = 0; i < 256; ++i) TEST_ASSERT_EQUAL(i, g[uint8_t(i)]);
}

static void test_gamma_22(void) {
    GammaLut g(2.2f);
    TEST_ASSERT_EQUAL(0, g[0]);
    TEST_ASSERT_EQUAL(255, g[255]);
    TEST_ASSERT_EQUAL(56, g[128]);  // (128/255)^2.2 * 255 = 55.5
    TEST_ASSERT_EQUAL(1, g[1]);     // dim pixels stay lit
}

static void test_monotonic(void) {
    GammaLut g(2.8f);
    for (int i = 1; i < 256; ++i) TEST_ASSERT_TRUE(g[uint8_t(i)] >= g[uint8_t(i - 1)]);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_identity);
    RUN_TEST(test_gamma_22);
    RUN_TEST(test_monotonic);
    return UNITY_END();
}
