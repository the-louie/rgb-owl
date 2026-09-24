#include <unity.h>

#include "owl/release.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static const ReleaseEntry LIST[] = {
    {"v1.1.0", false, false},
    {"v1.3.0-rc.1", true, false},
    {"v1.2.0", false, false},
    {"v2.0.0", false, true},  // draft
    {"nightly", true, false},
};

static void test_stable_channel(void) { TEST_ASSERT_EQUAL(2, pickRelease(LIST, 5, false)); }

static void test_prerelease_channel(void) { TEST_ASSERT_EQUAL(1, pickRelease(LIST, 5, true)); }

static void test_empty(void) {
    TEST_ASSERT_EQUAL(-1, pickRelease(LIST, 0, true));
    const ReleaseEntry only[] = {{"v1.0.0-beta", true, false}};
    TEST_ASSERT_EQUAL(-1, pickRelease(only, 1, false));
}

static void test_is_newer(void) {
    TEST_ASSERT_TRUE(isNewer("v1.2.0", "1.1.9"));
    TEST_ASSERT_FALSE(isNewer("v1.2.0", "1.2.0"));
    TEST_ASSERT_FALSE(isNewer("v1.2.0", "1.2.1-dev.3+abc"));  // dev build after 1.2.0 stays
    TEST_ASSERT_TRUE(isNewer("v1.2.1", "1.2.1-dev.3+abc"));
    TEST_ASSERT_TRUE(isNewer("v0.1.0", "0.0.0-dev+abc-dirty"));
    TEST_ASSERT_FALSE(isNewer("junk", "1.0.0"));
}

static void test_asset_kind(void) {
    TEST_ASSERT_TRUE(assetKind("owl-firmware-1.2.3.bin") == Asset::Image);
    TEST_ASSERT_TRUE(assetKind("owl-firmware-1.2.3.bin.sig") == Asset::Signature);
    TEST_ASSERT_TRUE(assetKind("owl-firmware.bin") == Asset::Image);  // v1.0.0 naming
    TEST_ASSERT_TRUE(assetKind("owl-firmware.bin.sig") == Asset::Signature);
    TEST_ASSERT_TRUE(assetKind("owl-firmware-1.3.0-rc.1.bin") == Asset::Image);
    TEST_ASSERT_TRUE(assetKind("owl-s3zero-merged-1.2.3.bin") == Asset::Other);
    TEST_ASSERT_TRUE(assetKind("owl-app-1.2.3.apk") == Asset::Other);
    TEST_ASSERT_TRUE(assetKind("owl-firmwarex.bin") == Asset::Other);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_stable_channel);
    RUN_TEST(test_prerelease_channel);
    RUN_TEST(test_empty);
    RUN_TEST(test_is_newer);
    RUN_TEST(test_asset_kind);
    return UNITY_END();
}
