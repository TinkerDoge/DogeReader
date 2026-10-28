#pragma once
#include <GfxRenderer.h>
#include <I18n.h>

#include <cstdint>
#include <cstdio>

// Shared formatting and shading for the reading-stats screens and the home
// screen summary line, so all of them read the same way.
namespace ReadingStatsFormat {

// "45 min" or "2h 05m".
inline void formatDuration(char* buf, const size_t size, const uint32_t minutes) {
  if (minutes < 60) {
    snprintf(buf, size, tr(STR_STATS_MINUTES_FORMAT), static_cast<unsigned>(minutes));
  } else {
    snprintf(buf, size, tr(STR_STATS_HOURS_MINUTES_FORMAT), static_cast<unsigned>(minutes / 60),
             static_cast<unsigned>(minutes % 60));
  }
}

// "1 day" or "12 days".
inline void formatDayCount(char* buf, const size_t size, const uint32_t days) {
  if (days == 1) {
    snprintf(buf, size, "%s", tr(STR_STATS_ONE_DAY));
  } else {
    snprintf(buf, size, tr(STR_STATS_DAYS_FORMAT), static_cast<unsigned>(days));
  }
}

inline const char* weekdayShort(const uint32_t mondayFirstIndex) {
  static constexpr StrId WEEKDAYS[7] = {StrId::STR_WEEKDAY_MON, StrId::STR_WEEKDAY_TUE, StrId::STR_WEEKDAY_WED,
                                        StrId::STR_WEEKDAY_THU, StrId::STR_WEEKDAY_FRI, StrId::STR_WEEKDAY_SAT,
                                        StrId::STR_WEEKDAY_SUN};
  return I18n::getInstance().get(WEEKDAYS[mondayFirstIndex % 7]);
}

inline const char* monthName(const uint32_t month) {
  static constexpr StrId MONTHS[12] = {StrId::STR_MONTH_JANUARY, StrId::STR_MONTH_FEBRUARY, StrId::STR_MONTH_MARCH,
                                       StrId::STR_MONTH_APRIL,   StrId::STR_MONTH_MAY,      StrId::STR_MONTH_JUNE,
                                       StrId::STR_MONTH_JULY,    StrId::STR_MONTH_AUGUST,   StrId::STR_MONTH_SEPTEMBER,
                                       StrId::STR_MONTH_OCTOBER, StrId::STR_MONTH_NOVEMBER, StrId::STR_MONTH_DECEMBER};
  return I18n::getInstance().get(MONTHS[(month + 11) % 12]);
}

// Fill for a heat level from ReadingStatsAnalytics::getHeatLevel: none, light,
// dark, black (goal met). Callers draw their own outline.
inline void fillHeat(const GfxRenderer& renderer, const int x, const int y, const int width, const int height,
                     const uint8_t level) {
  switch (level) {
    case 0:
      break;
    case 1:
      renderer.fillRectDither(x, y, width, height, Color::LightGray);
      break;
    case 2:
      renderer.fillRectDither(x, y, width, height, Color::DarkGray);
      break;
    default:
      renderer.fillRect(x, y, width, height, true);
      break;
  }
}

}  // namespace ReadingStatsFormat
