#include <unity.h>

#include "DebouncedButton.h"

void setUp() {}
void tearDown() {}

void test_press_and_release_each_emit_once() {
    DebouncedButton button;
    button.begin(false, 0);
    TEST_ASSERT_FALSE(button.update(true, 10));
    TEST_ASSERT_FALSE(button.update(true, 34));
    TEST_ASSERT_TRUE(button.update(true, 35));
    TEST_ASSERT_FALSE(button.update(true, 1000));
    TEST_ASSERT_FALSE(button.update(false, 1010));
    TEST_ASSERT_FALSE(button.update(false, 1034));
    TEST_ASSERT_TRUE(button.update(false, 1035));
    TEST_ASSERT_FALSE(button.update(false, 2000));
}

void test_bounce_restarts_stability_timer_on_both_edges() {
    DebouncedButton button;
    button.begin(false, 0);
    TEST_ASSERT_FALSE(button.update(true, 1));
    TEST_ASSERT_FALSE(button.update(false, 5));
    TEST_ASSERT_FALSE(button.update(true, 10));
    TEST_ASSERT_FALSE(button.update(true, 34));
    TEST_ASSERT_TRUE(button.update(true, 35));
    TEST_ASSERT_FALSE(button.update(false, 40));
    TEST_ASSERT_FALSE(button.update(true, 45));
    TEST_ASSERT_FALSE(button.update(false, 50));
    TEST_ASSERT_FALSE(button.update(false, 74));
    TEST_ASSERT_TRUE(button.update(false, 75));
}

void test_initial_state_does_not_emit_an_event() {
    DebouncedButton button;
    button.begin(false, 0);
    TEST_ASSERT_FALSE(button.update(false, 100));
    button.begin(true, 200);
    TEST_ASSERT_FALSE(button.update(true, 300));
    TEST_ASSERT_FALSE(button.update(false, 310));
    TEST_ASSERT_TRUE(button.update(false, 335));
}

void test_short_pulse_is_ignored() {
    DebouncedButton button;
    button.begin(false, 0);
    TEST_ASSERT_FALSE(button.update(true, 10));
    TEST_ASSERT_FALSE(button.update(false, 20));
    TEST_ASSERT_FALSE(button.update(false, 100));
}

void test_millis_wraparound() {
    DebouncedButton button;
    button.begin(false, UINT32_MAX - 20);
    TEST_ASSERT_FALSE(button.update(true, UINT32_MAX - 10));
    TEST_ASSERT_FALSE(button.update(true, 13));
    TEST_ASSERT_TRUE(button.update(true, 14));
    TEST_ASSERT_FALSE(button.update(true, 15));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_press_and_release_each_emit_once);
    RUN_TEST(test_bounce_restarts_stability_timer_on_both_edges);
    RUN_TEST(test_initial_state_does_not_emit_an_event);
    RUN_TEST(test_short_pulse_is_ignored);
    RUN_TEST(test_millis_wraparound);
    return UNITY_END();
}