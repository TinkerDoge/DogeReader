#include "ReadingStatsAnalytics.h"

#include <algorithm>

namespace ReadingStatsAnalytics {

uint8_t getHeatLevel(const uint32_t minutes, uint32_t goalMinutes) {
  if (minutes == 0) return 0;
  if (goalMinutes == 0) goalMinutes = 1;
  if (minutes >= goalMinutes) return 3;
  if (minutes * 2 >= goalMinutes) return 2;
  return 1;
}

uint32_t getCurrentStreak(const std::vector<ReadingDay>& days, const int32_t today) {
  int newestIndex = static_cast<int>(days.size()) - 1;
  while (newestIndex >= 0 && days[newestIndex].minutes == 0) newestIndex--;
  if (newestIndex < 0) return 0;

  const int32_t newest = days[newestIndex].day;
  // A streak is still alive if the newest reading day is today or yesterday.
  // With an unknown clock (today < 0) or a clock that went backwards, trust the
  // newest entry rather than reporting a broken streak.
  if (today >= 0 && newest <= today && newest < today - 1) return 0;

  uint32_t streak = 0;
  int32_t expected = newest;
  for (int i = newestIndex; i >= 0; i--) {
    if (days[i].minutes == 0 || days[i].day > expected) continue;
    if (days[i].day != expected) break;
    streak++;
    expected--;
  }
  return streak;
}

uint32_t getBestStreak(const std::vector<ReadingDay>& days) {
  uint32_t best = 0;
  uint32_t run = 0;
  int32_t previous = -2;
  for (const auto& entry : days) {
    if (entry.minutes == 0 || entry.day == previous) continue;
    run = (entry.day == previous + 1) ? run + 1 : 1;
    best = std::max(best, run);
    previous = entry.day;
  }
  return best;
}

uint16_t getMinutesOnDay(const std::vector<ReadingDay>& days, const int32_t day) {
  if (day < 0) return 0;
  const auto it = std::lower_bound(days.begin(), days.end(), day,
                                   [](const ReadingDay& entry, int32_t value) { return entry.day < value; });
  return (it != days.end() && it->day == day) ? it->minutes : 0;
}

int32_t daysFromCivil(int32_t year, const uint32_t month, const uint32_t dayOfMonth) {
  year -= month <= 2 ? 1 : 0;
  const int32_t era = (year >= 0 ? year : year - 399) / 400;
  const uint32_t yearOfEra = static_cast<uint32_t>(year - era * 400);
  const uint32_t dayOfYear = (153 * (month > 2 ? month - 3 : month + 9) + 2) / 5 + dayOfMonth - 1;
  const uint32_t dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
  return era * 146097 + static_cast<int32_t>(dayOfEra) - 719468;
}

void civilFromDays(int32_t days, int32_t& year, uint32_t& month, uint32_t& dayOfMonth) {
  days += 719468;
  const int32_t era = (days >= 0 ? days : days - 146096) / 146097;
  const uint32_t dayOfEra = static_cast<uint32_t>(days - era * 146097);
  const uint32_t yearOfEra = (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
  const uint32_t dayOfYear = dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
  const uint32_t monthIndex = (5 * dayOfYear + 2) / 153;
  dayOfMonth = dayOfYear - (153 * monthIndex + 2) / 5 + 1;
  month = monthIndex < 10 ? monthIndex + 3 : monthIndex - 9;
  year = static_cast<int32_t>(yearOfEra) + era * 400 + (month <= 2 ? 1 : 0);
}

uint32_t weekdayMondayFirst(const int32_t days) {
  // 1970-01-01 was a Thursday (index 3 when Monday is 0).
  return static_cast<uint32_t>(((days % 7) + 7 + 3) % 7);
}

uint32_t daysInMonth(const int32_t year, const uint32_t month) {
  static constexpr uint8_t DAYS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month < 1 || month > 12) return 0;
  const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
  return DAYS[month - 1] + ((month == 2 && leap) ? 1 : 0);
}

}  // namespace ReadingStatsAnalytics
