#include <unity.h>

#include "owl/pairing.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_new_phone_only_in_window(void) {
    TEST_ASSERT_TRUE(pairingAllowed(false, 0, 180000));
    TEST_ASSERT_TRUE(pairingAllowed(false, 179999, 180000));
    TEST_ASSERT_FALSE(pairingAllowed(false, 180000, 180000));
}

static void test_bonded_phone_always(void) {
    TEST_ASSERT_TRUE(pairingAllowed(true, 180000, 180000));
    TEST_ASSERT_TRUE(pairingAllowed(true, 0xFFFFFFFFu, 180000));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_new_phone_only_in_window);
    RUN_TEST(test_bonded_phone_always);
    return UNITY_END();
}
