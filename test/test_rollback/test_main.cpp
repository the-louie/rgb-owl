#include <unity.h>

#include "owl/rollback.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_needs_ble_and_grace(void) {
    TEST_ASSERT_FALSE(shouldMarkValid(false, 120000, 60000));
    TEST_ASSERT_FALSE(shouldMarkValid(true, 59999, 60000));
    TEST_ASSERT_TRUE(shouldMarkValid(true, 60000, 60000));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_needs_ble_and_grace);
    return UNITY_END();
}
