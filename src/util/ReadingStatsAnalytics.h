#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "../ReadingStatsStore.h"

class ReadingStatsAnalytics {
 public:
  static uint8_t getHeatLevel(uint32_t minutes);
  static uint32_t getCurrentStreak(const std::vector<ReadingDayStats>& history);
  static uint32_t getBestStreak(const std::vector<ReadingDayStats>& history);
  
  // Helper to get stats for the last 42 days (7x6 grid)
  static std::vector<uint32_t> getLast42DaysMinutes(const std::vector<ReadingDayStats>& history);
};
