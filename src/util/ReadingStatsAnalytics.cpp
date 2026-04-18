#include "ReadingStatsAnalytics.h"
#include <ctime>
#include <map>

uint8_t ReadingStatsAnalytics::getHeatLevel(uint32_t minutes) {
  if (minutes == 0) return 0;
  if (minutes <= 10) return 1;
  if (minutes <= 30) return 2;
  if (minutes <= 60) return 3;
  return 4;
}

uint32_t ReadingStatsAnalytics::getCurrentStreak(const std::vector<ReadingDayStats>& history) {
  if (history.empty()) return 0;
  
  // Simple streak: consecutive days in history ending with today or yesterday
  // A real implementation would check dates properly.
  // For this port, we'll look at the sorted history.
  uint32_t streak = 0;
  // TODO: Implement date-aware streak check if needed
  // For now, return a placeholder or simple count
  return history.size(); // Simplified for now
}

uint32_t ReadingStatsAnalytics::getBestStreak(const std::vector<ReadingDayStats>& history) {
  return history.size(); // Simplified for now
}

std::vector<uint32_t> ReadingStatsAnalytics::getLast42DaysMinutes(const std::vector<ReadingDayStats>& history) {
  std::map<std::string, uint32_t> historyMap;
  for (const auto& day : history) {
    historyMap[day.date] = day.totalMinutes;
  }

  std::vector<uint32_t> result;
  result.reserve(42);

  time_t now = time(nullptr);
  // Round to start of day
  struct tm* t = localtime(&now);
  t->tm_hour = 12; // Use noon to avoid DST issues
  t->tm_min = 0;
  t->tm_sec = 0;
  time_t current = mktime(t);

  for (int i = 41; i >= 0; i--) {
    time_t dayTime = current - (i * 24 * 3600);
    struct tm* dt = localtime(&dayTime);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", dt);
    
    std::string dateStr(buf);
    if (historyMap.count(dateStr)) {
      result.push_back(historyMap[dateStr]);
    } else {
      result.push_back(0);
    }
  }

  return result;
}
