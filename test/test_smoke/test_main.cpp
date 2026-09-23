#include <unity.h>

void setUp() {}
void tearDown() {}

static void test_toolchain(void) { TEST_ASSERT_EQUAL(4, sizeof(int)); }

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_toolchain);
    return UNITY_END();
}
