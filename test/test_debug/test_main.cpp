#include <string.h>
#include <unity.h>

#include "owl/ringlog.h"
#include "owl/stats.h"

using namespace owl;

void setUp() {}
void tearDown() {}

static void test_ringlog_keeps_all_until_full(void) {
    RingLog<16> r;
    r.append("ab\n", 3);
    r.append("cd\n", 3);
    char out[16];
    size_t n = r.copy(out);
    TEST_ASSERT_EQUAL(6, n);
    TEST_ASSERT_EQUAL_MEMORY("ab\ncd\n", out, 6);
}

static void test_ringlog_wraps_to_line_boundary(void) {
    RingLog<8> r;
    r.append("111\n222\n333\n", 12);  // holds "222\n333\n"; the oldest line may be partial, so it is dropped
    char out[8];
    size_t n = r.copy(out);
    TEST_ASSERT_EQUAL(4, n);
    TEST_ASSERT_EQUAL_MEMORY("333\n", out, 4);
}

static void test_stats_fps_and_max(void) {
    FrameStats s;
    for (uint32_t t = 0; t <= 1000; t += 20) s.record(t == 500 ? 9000 : 1000, t);
    TEST_ASSERT_EQUAL(51, s.fps());
    TEST_ASSERT_EQUAL(9000, s.maxFrameUs());
}

static void test_stats_zero_before_first_window(void) {
    FrameStats s;
    s.record(100, 0);
    s.record(100, 500);
    TEST_ASSERT_EQUAL(0, s.fps());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_ringlog_keeps_all_until_full);
    RUN_TEST(test_ringlog_wraps_to_line_boundary);
    RUN_TEST(test_stats_fps_and_max);
    RUN_TEST(test_stats_zero_before_first_window);
    return UNITY_END();
}
