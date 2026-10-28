#pragma once
#include <cstdint>
#include <vector>

// One calendar day of reading. `day` is the number of days since 1970-01-01 in the
// reader's local time zone (see ReadingStatsStore::currentLocalDay), which fits a
// uint16_t until the year 2149.
struct ReadingDay {
  uint16_t day;
  uint16_t minutes;
};

// Pure functions over a day-sorted history: no clock, filesystem or settings
// access, so everything here is deterministic and host-testable.
namespace ReadingStatsAnalytics {

// Heat bucket for the calendar, relative to the daily goal:
// 0 = no reading, 1 = under half the goal, 2 = at least half, 3 = goal met.
uint8_t getHeatLevel(uint32_t minutes, uint32_t goalMinutes);

// Consecutive reading days ending today (or yesterday, if today has no reading
// yet). When `today` is negative (clock unknown) or earlier than the newest
// entry (clock went backwards), the streak is anchored on the newest entry.
uint32_t getCurrentStreak(const std::vector<ReadingDay>& days, int32_t today);

// Longest run of consecutive reading days anywhere in `days`.
uint32_t getBestStreak(const std::vector<ReadingDay>& days);

// Minutes read on `day`, 0 if absent. `days` must be sorted ascending.
uint16_t getMinutesOnDay(const std::vector<ReadingDay>& days, int32_t day);

// Proleptic Gregorian calendar conversions (Howard Hinnant's algorithms).
int32_t daysFromCivil(int32_t year, uint32_t month, uint32_t dayOfMonth);
void civilFromDays(int32_t days, int32_t& year, uint32_t& month, uint32_t& dayOfMonth);
// 0 = Monday ... 6 = Sunday.
uint32_t weekdayMondayFirst(int32_t days);
uint32_t daysInMonth(int32_t year, uint32_t month);

}  // namespace ReadingStatsAnalytics
