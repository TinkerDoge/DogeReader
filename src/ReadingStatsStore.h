#pragma once
#include <cstdint>
#include <vector>

#include "util/ReadingStatsAnalytics.h"

// Reading time and progress, persisted as a small fixed-layout binary file.
//
// RAM budget: one 4-byte entry per reading day (capped at MAX_DAYS) plus one
// 12-byte entry per book (capped at MAX_BOOKS) -> at most ~2.2 KB of heap, no
// strings. Books are keyed by a stable FNV-1a hash of their path.
//
// Dates use the user's configured UTC offset (Settings > Status Bar > UTC offset).
// When the system clock is unset (X4 has no RTC, so it resets on every power-off
// until Wi-Fi syncs it), minutes still count toward lifetime and per-book totals
// but are not attributed to a calendar day.
class ReadingStatsStore {
 public:
  struct BookStats {
    uint32_t pathHash;
    uint32_t minutes;
    uint8_t percent;  // last known progress, 0-100
  };

  static constexpr uint16_t MAX_DAYS = 400;
  static constexpr uint16_t MAX_BOOKS = 64;
  static constexpr uint16_t DEFAULT_DAILY_GOAL_MINUTES = 30;

  static ReadingStatsStore& getInstance() { return instance; }

  bool loadFromFile();

  // Record one finished reading session: `minutes` of active reading and the
  // last known progress (`percent` < 0 when unknown). Saves at most once, and
  // only if something changed.
  void recordSession(const char* bookPath, uint32_t minutes, int percent);

  const std::vector<ReadingDay>& getDays() const { return days; }
  uint32_t getLifetimeMinutes() const { return lifetimeMinutes; }
  uint16_t getDailyGoalMinutes() const { return dailyGoalMinutes; }
  uint32_t getTodayMinutes() const;
  uint32_t getCurrentStreak() const;
  uint32_t getBestStreak() const;
  // Stats for a book, or nullptr if it has never been read.
  const BookStats* findBook(const char* bookPath) const;

  // Today's day number in local time, or -1 if the system clock is unset.
  static int32_t currentLocalDay();
  static uint32_t hashPath(const char* path);

 private:
  static ReadingStatsStore instance;

  // Sorted ascending by day, unique days only.
  std::vector<ReadingDay> days;
  // Most recently read first; the tail is evicted when MAX_BOOKS is exceeded.
  std::vector<BookStats> books;
  uint32_t lifetimeMinutes = 0;
  // Persisted so trimming old days never lowers the best streak.
  uint16_t storedBestStreak = 0;
  uint16_t dailyGoalMinutes = DEFAULT_DAILY_GOAL_MINUTES;
  bool loaded = false;

  ReadingStatsStore() = default;

  void addDayMinutes(int32_t day, uint32_t minutes);
  // Returns the book's entry, moved to the front (created if absent).
  BookStats& touchBook(uint32_t pathHash);
  bool saveToFile() const;
  bool loadBinary(const char* path);
  bool migrateLegacyJson();
};

#define READING_STATS ReadingStatsStore::getInstance()
