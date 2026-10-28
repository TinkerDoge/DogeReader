#include "ReadingStatsStore.h"

#include <ArduinoJson.h>
#include <HalClock.h>
#include <HalStorage.h>
#include <Logging.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "CrossPointSettings.h"
#include "util/HalFileJsonReader.h"

ReadingStatsStore ReadingStatsStore::instance;

namespace {
constexpr char STATS_DIR[] = "/.crosspoint";
constexpr char STATS_PATH[] = "/.crosspoint/reading_stats.bin";
constexpr char STATS_TMP_PATH[] = "/.crosspoint/reading_stats.tmp";
constexpr char STATS_BAD_PATH[] = "/.crosspoint/reading_stats.bad";
constexpr char LEGACY_JSON_PATH[] = "/stats/reading_stats.json";
constexpr char LEGACY_JSON_BACKUP_PATH[] = "/stats/reading_stats.json.bak";

// File layout (little-endian, packed, read via memcpy for RISC-V alignment safety):
//   0  char[4]  magic "DRST"
//   4  u8       version
//   5  u8[3]    reserved
//   8  u32      lifetime minutes
//   12 u16      best streak (historic, survives day trimming)
//   14 u16      daily goal minutes
//   16 u16      day count
//   18 u16      book count
//   20 day count  x { u16 day, u16 minutes }
//   .. book count x { u32 path hash, u32 minutes, u8 percent, u8[3] reserved }
constexpr char MAGIC[4] = {'D', 'R', 'S', 'T'};
constexpr uint8_t FILE_VERSION = 1;
constexpr size_t HEADER_SIZE = 20;
constexpr size_t DAY_SIZE = 4;
constexpr size_t BOOK_SIZE = 12;
constexpr uint32_t MINUTES_PER_DAY = 24 * 60;
// Before this, dates come from an unset clock (time(nullptr) starts near 0).
constexpr int32_t MIN_VALID_YEAR = 2023;

uint8_t clampPercent(const int percent) { return static_cast<uint8_t>(std::clamp(percent, 0, 100)); }
}  // namespace

uint32_t ReadingStatsStore::hashPath(const char* path) {
  // FNV-1a: stable across builds, unlike std::hash.
  uint32_t hash = 2166136261u;
  for (const char* p = path; *p != '\0'; ++p) {
    hash ^= static_cast<uint8_t>(*p);
    hash *= 16777619u;
  }
  return hash;
}

int32_t ReadingStatsStore::currentLocalDay() {
  if (!HalClock::systemTimeValid()) return -1;
  uint8_t offsetQuarterHours = SETTINGS.clockUtcOffsetQ;
  if (offsetQuarterHours > 104) offsetQuarterHours = 48;  // 48 = UTC+0
  const int64_t offsetSeconds = (static_cast<int64_t>(offsetQuarterHours) - 48) * 15 * 60;
  return static_cast<int32_t>((static_cast<int64_t>(time(nullptr)) + offsetSeconds) / 86400);
}

uint32_t ReadingStatsStore::getTodayMinutes() const {
  return ReadingStatsAnalytics::getMinutesOnDay(days, currentLocalDay());
}

uint32_t ReadingStatsStore::getCurrentStreak() const {
  return ReadingStatsAnalytics::getCurrentStreak(days, currentLocalDay());
}

uint32_t ReadingStatsStore::getBestStreak() const {
  return std::max<uint32_t>(storedBestStreak, ReadingStatsAnalytics::getBestStreak(days));
}

const ReadingStatsStore::BookStats* ReadingStatsStore::findBook(const char* bookPath) const {
  if (bookPath == nullptr) return nullptr;
  const uint32_t hash = hashPath(bookPath);
  const auto it =
      std::find_if(books.begin(), books.end(), [hash](const BookStats& book) { return book.pathHash == hash; });
  return it != books.end() ? &*it : nullptr;
}

void ReadingStatsStore::recordSession(const char* bookPath, uint32_t minutes, const int percent) {
  if (bookPath == nullptr || bookPath[0] == '\0') return;
  if (!loaded) loadFromFile();

  minutes = std::min(minutes, MINUTES_PER_DAY);
  const BookStats* existing = findBook(bookPath);
  const bool percentChanged = percent >= 0 && (existing == nullptr || existing->percent != clampPercent(percent));
  if (minutes == 0 && !percentChanged) return;  // nothing new, skip the SD write

  BookStats& book = touchBook(hashPath(bookPath));
  book.minutes += minutes;
  if (percent >= 0) book.percent = clampPercent(percent);
  lifetimeMinutes += minutes;
  if (minutes > 0) addDayMinutes(currentLocalDay(), minutes);

  saveToFile();
}

void ReadingStatsStore::addDayMinutes(const int32_t day, const uint32_t minutes) {
  if (day < 0 || day > UINT16_MAX || minutes == 0) return;
  const auto byDay = [](const ReadingDay& entry, int32_t value) { return entry.day < value; };

  auto it = std::lower_bound(days.begin(), days.end(), day, byDay);
  if (it != days.end() && it->day == day) {
    it->minutes = static_cast<uint16_t>(std::min<uint32_t>(it->minutes + minutes, MINUTES_PER_DAY));
    return;
  }

  if (days.size() >= MAX_DAYS) {
    if (it == days.begin()) return;  // older than everything kept
    storedBestStreak = static_cast<uint16_t>(getBestStreak());
    days.erase(days.begin());
    it = std::lower_bound(days.begin(), days.end(), day, byDay);
  }
  days.insert(it, ReadingDay{static_cast<uint16_t>(day), static_cast<uint16_t>(minutes)});
}

ReadingStatsStore::BookStats& ReadingStatsStore::touchBook(const uint32_t pathHash) {
  BookStats entry{pathHash, 0, 0};
  const auto it =
      std::find_if(books.begin(), books.end(), [pathHash](const BookStats& book) { return book.pathHash == pathHash; });
  if (it != books.end()) {
    entry = *it;
    books.erase(it);
  } else if (books.size() >= MAX_BOOKS) {
    books.pop_back();
  }
  books.insert(books.begin(), entry);
  return books.front();
}

bool ReadingStatsStore::saveToFile() const {
  Storage.ensureDirectoryExists(STATS_DIR);

  {
    HalFile file;
    if (!Storage.openFileForWrite("RSS", STATS_TMP_PATH, file)) {
      LOG_ERR("RSS", "Failed to open %s for writing", STATS_TMP_PATH);
      return false;
    }

    uint8_t header[HEADER_SIZE] = {};
    const uint16_t bestStreak = static_cast<uint16_t>(std::min<uint32_t>(getBestStreak(), UINT16_MAX));
    const uint16_t dayCount = static_cast<uint16_t>(days.size());
    const uint16_t bookCount = static_cast<uint16_t>(books.size());
    memcpy(header, MAGIC, sizeof(MAGIC));
    header[4] = FILE_VERSION;
    memcpy(header + 8, &lifetimeMinutes, sizeof(lifetimeMinutes));
    memcpy(header + 12, &bestStreak, sizeof(bestStreak));
    memcpy(header + 14, &dailyGoalMinutes, sizeof(dailyGoalMinutes));
    memcpy(header + 16, &dayCount, sizeof(dayCount));
    memcpy(header + 18, &bookCount, sizeof(bookCount));
    bool ok = file.write(header, HEADER_SIZE) == HEADER_SIZE;

    for (const auto& entry : days) {
      uint8_t raw[DAY_SIZE];
      memcpy(raw, &entry.day, 2);
      memcpy(raw + 2, &entry.minutes, 2);
      ok = ok && file.write(raw, DAY_SIZE) == DAY_SIZE;
    }
    for (const auto& book : books) {
      uint8_t raw[BOOK_SIZE] = {};
      memcpy(raw, &book.pathHash, 4);
      memcpy(raw + 4, &book.minutes, 4);
      raw[8] = book.percent;
      ok = ok && file.write(raw, BOOK_SIZE) == BOOK_SIZE;
    }

    if (!ok) {
      LOG_ERR("RSS", "Short write to %s", STATS_TMP_PATH);
      file.close();  // close before delete
      Storage.remove(STATS_TMP_PATH);
      return false;
    }
  }  // file closed here, before the rename

  // FAT rename cannot overwrite. If power is lost between these two calls the
  // temp file survives and loadFromFile() picks it up.
  Storage.remove(STATS_PATH);
  if (!Storage.rename(STATS_TMP_PATH, STATS_PATH)) {
    LOG_ERR("RSS", "Failed to move %s into place", STATS_TMP_PATH);
    return false;
  }
  return true;
}

bool ReadingStatsStore::loadFromFile() {
  loaded = true;

  if (Storage.exists(STATS_PATH)) {
    if (loadBinary(STATS_PATH)) return true;
    // Keep the unreadable file for inspection instead of overwriting it on the next save.
    Storage.remove(STATS_BAD_PATH);
    Storage.rename(STATS_PATH, STATS_BAD_PATH);
    return false;
  }
  if (Storage.exists(STATS_TMP_PATH)) return loadBinary(STATS_TMP_PATH);
  if (Storage.exists(LEGACY_JSON_PATH) && migrateLegacyJson()) {
    saveToFile();
    Storage.rename(LEGACY_JSON_PATH, LEGACY_JSON_BACKUP_PATH);
    return true;
  }
  return false;
}

bool ReadingStatsStore::loadBinary(const char* path) {
  HalFile file;
  if (!Storage.openFileForRead("RSS", path, file)) return false;

  uint8_t header[HEADER_SIZE];
  if (file.read(header, HEADER_SIZE) != static_cast<int>(HEADER_SIZE) || memcmp(header, MAGIC, sizeof(MAGIC)) != 0 ||
      header[4] != FILE_VERSION) {
    LOG_ERR("RSS", "Unrecognized stats file %s, starting fresh", path);
    return false;
  }

  uint32_t fileLifetime;
  uint16_t fileBestStreak, fileGoal, dayCount, bookCount;
  memcpy(&fileLifetime, header + 8, 4);
  memcpy(&fileBestStreak, header + 12, 2);
  memcpy(&fileGoal, header + 14, 2);
  memcpy(&dayCount, header + 16, 2);
  memcpy(&bookCount, header + 18, 2);
  if (dayCount > MAX_DAYS || bookCount > MAX_BOOKS) {
    LOG_ERR("RSS", "Stats file %s has out-of-range counts (%u days, %u books)", path, dayCount, bookCount);
    return false;
  }

  // Headroom so appending a day or a book does not immediately reallocate.
  std::vector<ReadingDay> fileDays;
  fileDays.reserve(std::min<size_t>(dayCount + 16u, MAX_DAYS));
  for (uint16_t i = 0; i < dayCount; i++) {
    uint8_t raw[DAY_SIZE];
    if (file.read(raw, DAY_SIZE) != static_cast<int>(DAY_SIZE)) {
      LOG_ERR("RSS", "Stats file %s truncated in day table", path);
      return false;
    }
    ReadingDay entry;
    memcpy(&entry.day, raw, 2);
    memcpy(&entry.minutes, raw + 2, 2);
    if (!fileDays.empty() && entry.day <= fileDays.back().day) {
      LOG_ERR("RSS", "Stats file %s has unsorted days", path);
      return false;
    }
    fileDays.push_back(entry);
  }

  std::vector<BookStats> fileBooks;
  fileBooks.reserve(std::min<size_t>(bookCount + 4u, MAX_BOOKS));
  for (uint16_t i = 0; i < bookCount; i++) {
    uint8_t raw[BOOK_SIZE];
    if (file.read(raw, BOOK_SIZE) != static_cast<int>(BOOK_SIZE)) {
      LOG_ERR("RSS", "Stats file %s truncated in book table", path);
      return false;
    }
    BookStats book;
    memcpy(&book.pathHash, raw, 4);
    memcpy(&book.minutes, raw + 4, 4);
    book.percent = clampPercent(raw[8]);
    fileBooks.push_back(book);
  }

  days = std::move(fileDays);
  books = std::move(fileBooks);
  lifetimeMinutes = fileLifetime;
  storedBestStreak = fileBestStreak;
  dailyGoalMinutes = fileGoal > 0 ? fileGoal : DEFAULT_DAILY_GOAL_MINUTES;
  LOG_DBG("RSS", "Loaded reading stats: %u days, %u books, %lu min", dayCount, bookCount,
          static_cast<unsigned long>(lifetimeMinutes));
  return true;
}

bool ReadingStatsStore::migrateLegacyJson() {
  HalFile file;
  if (!Storage.openFileForRead("RSS", LEGACY_JSON_PATH, file)) return false;

  // Keep only what the new format stores; per-session records are dropped.
  JsonDocument filter;
  filter["totalMinutes"] = true;
  filter["dailyGoal"] = true;
  filter["history"][0]["date"] = true;
  filter["history"][0]["total"] = true;
  filter["history"][0]["books"] = true;
  filter["bookProgress"] = true;

  JsonDocument doc;
  HalFileJsonReader reader{file};
  const auto error = deserializeJson(doc, reader, DeserializationOption::Filter(filter));
  if (error) {
    LOG_ERR("RSS", "Legacy stats not migrated: %s", error.c_str());
    return false;
  }

  days.clear();
  books.clear();
  lifetimeMinutes = doc["totalMinutes"] | 0u;
  dailyGoalMinutes = doc["dailyGoal"] | DEFAULT_DAILY_GOAL_MINUTES;
  if (dailyGoalMinutes == 0) dailyGoalMinutes = DEFAULT_DAILY_GOAL_MINUTES;

  for (JsonObjectConst dayObj : doc["history"].as<JsonArrayConst>()) {
    int year = 0, month = 0, dayOfMonth = 0;
    const char* date = dayObj["date"] | "";
    const uint32_t total = dayObj["total"] | 0u;
    // Entries dated before the clock was ever set (1970-01-01) carry no real date.
    if (sscanf(date, "%4d-%2d-%2d", &year, &month, &dayOfMonth) == 3 && year >= MIN_VALID_YEAR && month >= 1 &&
        month <= 12 && dayOfMonth >= 1 && dayOfMonth <= 31) {
      addDayMinutes(ReadingStatsAnalytics::daysFromCivil(year, month, dayOfMonth), total);
    }
    for (JsonPairConst entry : dayObj["books"].as<JsonObjectConst>()) {
      touchBook(hashPath(entry.key().c_str())).minutes += entry.value().as<uint32_t>();
    }
  }
  for (JsonPairConst entry : doc["bookProgress"].as<JsonObjectConst>()) {
    touchBook(hashPath(entry.key().c_str())).percent = clampPercent(entry.value().as<int>());
  }

  LOG_INF("RSS", "Migrated legacy reading stats (%u days, %u books)", static_cast<unsigned>(days.size()),
          static_cast<unsigned>(books.size()));
  return true;
}
