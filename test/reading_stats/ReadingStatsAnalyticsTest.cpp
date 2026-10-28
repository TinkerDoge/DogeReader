/**
 * ReadingStatsAnalyticsTest.cpp
 *
 * Host unit tests for ReadingStatsAnalytics: streak math over day-sorted
 * history and the calendar conversions the heatmap relies on.
 *   - empty history, only today, consecutive runs, yesterday-alive, broken
 *   - gaps, duplicate days, zero-minute days
 *   - clock went backwards / clock unknown
 *   - month, year and leap-year boundaries
 */

#include <cstdio>
#include <vector>

#include "util/ReadingStatsAnalytics.h"

using namespace ReadingStatsAnalytics;

static int testsPassed = 0;
static int testsFailed = 0;

#define ASSERT_EQ(a, b)                                                                                        \
  do {                                                                                                         \
    auto _a = (a);                                                                                             \
    auto _b = (b);                                                                                             \
    if (_a != _b) {                                                                                            \
      fprintf(stderr, "  FAIL: %s:%d: %s == %ld, expected %ld\n", __FILE__, __LINE__, #a, (long)_a, (long)_b); \
      testsFailed++;                                                                                           \
      return;                                                                                                  \
    }                                                                                                          \
  } while (0)

#define PASS() testsPassed++

static int32_t D(int32_t y, uint32_t m, uint32_t d) { return daysFromCivil(y, m, d); }

static ReadingDay day(int32_t y, uint32_t m, uint32_t d, uint16_t minutes = 30) {
  return ReadingDay{static_cast<uint16_t>(D(y, m, d)), minutes};
}

// ---- calendar ----

void testCivilRoundTrip() {
  printf("  civil: epoch and round trips\n");
  ASSERT_EQ(D(1970, 1, 1), 0);
  ASSERT_EQ(D(2000, 3, 1), 11017);
  for (int32_t n = -1000; n < 80000; n += 37) {
    int32_t y;
    uint32_t m, d;
    civilFromDays(n, y, m, d);
    ASSERT_EQ(D(y, m, d), n);
  }
  PASS();
}

void testWeekday() {
  printf("  civil: weekday (Monday = 0)\n");
  ASSERT_EQ(weekdayMondayFirst(D(1970, 1, 1)), 3u);    // Thursday
  ASSERT_EQ(weekdayMondayFirst(D(2026, 10, 26)), 0u);  // Monday
  ASSERT_EQ(weekdayMondayFirst(D(2026, 11, 1)), 6u);   // Sunday
  PASS();
}

void testDaysInMonth() {
  printf("  civil: days in month incl. leap years\n");
  ASSERT_EQ(daysInMonth(2026, 2), 28u);
  ASSERT_EQ(daysInMonth(2028, 2), 29u);
  ASSERT_EQ(daysInMonth(2100, 2), 28u);
  ASSERT_EQ(daysInMonth(2000, 2), 29u);
  ASSERT_EQ(daysInMonth(2026, 4), 30u);
  ASSERT_EQ(daysInMonth(2026, 12), 31u);
  ASSERT_EQ(daysInMonth(2026, 13), 0u);
  PASS();
}

// ---- getCurrentStreak ----

void testEmptyHistory_currentStreak() {
  printf("  getCurrentStreak: empty history -> 0\n");
  ASSERT_EQ(getCurrentStreak({}, D(2026, 7, 20)), 0u);
  PASS();
}

void testOnlyToday_currentStreak() {
  printf("  getCurrentStreak: only today -> 1\n");
  ASSERT_EQ(getCurrentStreak({day(2026, 7, 20)}, D(2026, 7, 20)), 1u);
  PASS();
}

void testThreeConsecutiveDaysEndingToday() {
  printf("  getCurrentStreak: 3 consecutive days ending today -> 3\n");
  ASSERT_EQ(getCurrentStreak({day(2026, 7, 18), day(2026, 7, 19), day(2026, 7, 20)}, D(2026, 7, 20)), 3u);
  PASS();
}

void testReadYesterdayNotToday() {
  printf("  getCurrentStreak: read yesterday not today -> streak alive\n");
  ASSERT_EQ(getCurrentStreak({day(2026, 7, 17), day(2026, 7, 18), day(2026, 7, 19)}, D(2026, 7, 20)), 3u);
  PASS();
}

void testFullDaySkipped() {
  printf("  getCurrentStreak: full day skipped -> 0\n");
  ASSERT_EQ(getCurrentStreak({day(2026, 7, 17), day(2026, 7, 18)}, D(2026, 7, 20)), 0u);
  PASS();
}

void testGapInMiddle_currentStreak() {
  printf("  getCurrentStreak: gap in middle -> only latest run\n");
  ASSERT_EQ(getCurrentStreak({day(2026, 7, 14), day(2026, 7, 16), day(2026, 7, 17)}, D(2026, 7, 17)), 2u);
  PASS();
}

void testDuplicateDays_currentStreak() {
  printf("  getCurrentStreak: duplicate days -> counted once\n");
  ASSERT_EQ(getCurrentStreak({day(2026, 7, 18), day(2026, 7, 19), day(2026, 7, 19), day(2026, 7, 20)}, D(2026, 7, 20)),
            3u);
  PASS();
}

void testClockWentBackwards_currentStreak() {
  printf("  getCurrentStreak: clock went backwards -> anchors on newest day\n");
  ASSERT_EQ(getCurrentStreak({day(2026, 7, 20), day(2026, 7, 21), day(2026, 7, 22)}, D(2026, 7, 20)), 3u);
  PASS();
}

void testClockUnknown_currentStreak() {
  printf("  getCurrentStreak: clock unknown -> anchors on newest day\n");
  ASSERT_EQ(getCurrentStreak({day(2026, 7, 21), day(2026, 7, 22)}, -1), 2u);
  PASS();
}

void testZeroMinuteDay_currentStreak() {
  printf("  getCurrentStreak: day with 0 minutes -> not counted\n");
  ASSERT_EQ(getCurrentStreak({day(2026, 7, 18), day(2026, 7, 19, 0), day(2026, 7, 20)}, D(2026, 7, 20)), 1u);
  PASS();
}

// ---- getBestStreak ----

void testEmptyHistory_bestStreak() {
  printf("  getBestStreak: empty history -> 0\n");
  ASSERT_EQ(getBestStreak({}), 0u);
  PASS();
}

void testOnlyToday_bestStreak() {
  printf("  getBestStreak: single day -> 1\n");
  ASSERT_EQ(getBestStreak({day(2026, 7, 20)}), 1u);
  PASS();
}

void testGapInMiddle_bestStreak() {
  printf("  getBestStreak: earlier run longer than later run\n");
  ASSERT_EQ(getBestStreak({day(2026, 7, 1), day(2026, 7, 2), day(2026, 7, 3), day(2026, 7, 5), day(2026, 7, 6)}), 3u);
  PASS();
}

void testLaterRunIsLonger_bestStreak() {
  printf("  getBestStreak: later run longer than earlier run\n");
  ASSERT_EQ(getBestStreak({day(2026, 7, 1), day(2026, 7, 3), day(2026, 7, 4), day(2026, 7, 5), day(2026, 7, 6)}), 4u);
  PASS();
}

void testDuplicateDays_bestStreak() {
  printf("  getBestStreak: duplicate days -> counted once\n");
  ASSERT_EQ(getBestStreak({day(2026, 7, 1), day(2026, 7, 1), day(2026, 7, 2)}), 2u);
  PASS();
}

void testZeroMinuteDay_bestStreak() {
  printf("  getBestStreak: zero-minute day breaks the run\n");
  ASSERT_EQ(getBestStreak({day(2026, 7, 1), day(2026, 7, 2, 0), day(2026, 7, 3), day(2026, 7, 4)}), 2u);
  PASS();
}

void testMonthBoundary_bestStreak() {
  printf("  getBestStreak: run crossing a month boundary\n");
  ASSERT_EQ(getBestStreak({day(2026, 6, 29), day(2026, 6, 30), day(2026, 7, 1), day(2026, 7, 2)}), 4u);
  PASS();
}

void testYearBoundary_bestStreak() {
  printf("  getBestStreak: run crossing a year boundary\n");
  ASSERT_EQ(getBestStreak({day(2025, 12, 30), day(2025, 12, 31), day(2026, 1, 1), day(2026, 1, 2)}), 4u);
  PASS();
}

void testLeapDay_bestStreak() {
  printf("  getBestStreak: run across Feb 29\n");
  ASSERT_EQ(getBestStreak({day(2028, 2, 28), day(2028, 2, 29), day(2028, 3, 1)}), 3u);
  PASS();
}

// ---- lookups ----

void testMinutesOnDay() {
  printf("  getMinutesOnDay: hit, miss, unknown day\n");
  const std::vector<ReadingDay> days = {day(2026, 7, 1, 12), day(2026, 7, 3, 45)};
  ASSERT_EQ(getMinutesOnDay(days, D(2026, 7, 3)), 45);
  ASSERT_EQ(getMinutesOnDay(days, D(2026, 7, 2)), 0);
  ASSERT_EQ(getMinutesOnDay(days, -1), 0);
  PASS();
}

void testHeatLevels() {
  printf("  getHeatLevel: relative to a 30 minute goal\n");
  ASSERT_EQ(getHeatLevel(0, 30), 0);
  ASSERT_EQ(getHeatLevel(1, 30), 1);
  ASSERT_EQ(getHeatLevel(14, 30), 1);
  ASSERT_EQ(getHeatLevel(15, 30), 2);
  ASSERT_EQ(getHeatLevel(29, 30), 2);
  ASSERT_EQ(getHeatLevel(30, 30), 3);
  ASSERT_EQ(getHeatLevel(5, 0), 3);
  PASS();
}

int main() {
  printf("=== Reading Stats Analytics Tests ===\n\n");

  printf("calendar:\n");
  testCivilRoundTrip();
  testWeekday();
  testDaysInMonth();

  printf("\ngetCurrentStreak:\n");
  testEmptyHistory_currentStreak();
  testOnlyToday_currentStreak();
  testThreeConsecutiveDaysEndingToday();
  testReadYesterdayNotToday();
  testFullDaySkipped();
  testGapInMiddle_currentStreak();
  testDuplicateDays_currentStreak();
  testClockWentBackwards_currentStreak();
  testClockUnknown_currentStreak();
  testZeroMinuteDay_currentStreak();

  printf("\ngetBestStreak:\n");
  testEmptyHistory_bestStreak();
  testOnlyToday_bestStreak();
  testGapInMiddle_bestStreak();
  testLaterRunIsLonger_bestStreak();
  testDuplicateDays_bestStreak();
  testZeroMinuteDay_bestStreak();
  testMonthBoundary_bestStreak();
  testYearBoundary_bestStreak();
  testLeapDay_bestStreak();

  printf("\nlookups:\n");
  testMinutesOnDay();
  testHeatLevels();

  printf("\n=== Results: %d passed, %d failed ===\n", testsPassed, testsFailed);
  return testsFailed > 0 ? 1 : 0;
}
