#include "ReadingStatsStore.h"
#include <HalStorage.h>
#include <Logging.h>
#include <ctime>
#include <algorithm>
#include "JsonSettingsIO.h"

ReadingStatsStore ReadingStatsStore::instance;

const char* STATS_FILE_PATH = "/stats/reading_stats.json";

void ReadingStatsStore::addMinutes(uint32_t minutes, const std::string& bookPath) {
  findOrCreateToday();
  ReadingDayStats& today = history.back();
  today.totalMinutes += minutes;
  today.bookMinutes[bookPath] += minutes;
  
  ReadingSession session;
  session.bookPath = bookPath;
  session.startTime = time(nullptr) - (minutes * 60);
  session.minutes = minutes;
  today.sessions.push_back(session);
  
  lifetimeMinutes += minutes;

  // Update best streak
  uint32_t currentStreak = getCurrentStreak();
  if (currentStreak > bestStreak) {
    bestStreak = currentStreak;
  }

  saveToFile();
}

uint32_t ReadingStatsStore::getTodayMinutes() const {
  std::string todayDate = const_cast<ReadingStatsStore*>(this)->getTodayDate();
  if (!history.empty() && history.back().date == todayDate) {
    return history.back().totalMinutes;
  }
  return 0;
}

uint32_t ReadingStatsStore::getCurrentStreak() const {
  // Logic to calculate streak based on history
  // history is assumed to be sorted by date
  if (history.empty()) return 0;
  
  // TODO: More robust date comparison
  return history.size(); // Simplified
}

std::vector<ReadingDayStats> ReadingStatsStore::getStatsForMonth(int month, int year) const {
  std::vector<ReadingDayStats> monthStats;
  char prefix[8];
  snprintf(prefix, sizeof(prefix), "%04d-%02d", year, month);
  std::string prefixStr(prefix);

  for (const auto& day : history) {
    if (day.date.find(prefixStr) == 0) {
      monthStats.push_back(day);
    }
  }
  return monthStats;
}

void ReadingStatsStore::findOrCreateToday() {
  std::string todayDate = getTodayDate();
  if (history.empty() || history.back().date != todayDate) {
    history.push_back({todayDate, 0, {}, {}});
  }
}

std::string ReadingStatsStore::getTodayDate() {
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", t);
  return std::string(buf);
}

bool ReadingStatsStore::saveToFile() const {
  if (!Storage.exists("/stats")) {
    Storage.mkdir("/stats");
  }
  return JsonSettingsIO::saveReadingStats(*this, STATS_FILE_PATH);
}

bool ReadingStatsStore::loadFromFile() {
  if (!Storage.exists(STATS_FILE_PATH)) {
    return false;
  }
  String json = Storage.readFile(STATS_FILE_PATH);
  if (json.length() == 0) {
    return false;
  }
  return JsonSettingsIO::loadReadingStats(*this, json.c_str());
}
