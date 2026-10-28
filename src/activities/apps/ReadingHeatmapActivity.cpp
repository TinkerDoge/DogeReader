#include "ReadingHeatmapActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>

#include "ReadingStatsFormat.h"
#include "ReadingStatsStore.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr int CELL_GAP = 6;
constexpr int CELL_TEXT_INSET = 4;
constexpr int SECTION_GAP = 14;
constexpr int LEGEND_SWATCH = 18;
constexpr int LEGEND_ITEM_GAP = 18;
constexpr int MAX_WEEK_ROWS = 6;

// Today if the clock is set, otherwise the newest recorded day (-1 if none).
int32_t anchorDay() {
  const int32_t today = ReadingStatsStore::currentLocalDay();
  if (today >= 0) return today;
  const auto& days = READING_STATS.getDays();
  return days.empty() ? -1 : days.back().day;
}
}  // namespace

void ReadingHeatmapActivity::onEnter() {
  Activity::onEnter();
  showMonthOf(anchorDay());
  requestUpdate();
}

void ReadingHeatmapActivity::showMonthOf(const int32_t day) {
  if (day < 0) {
    month = 0;
    return;
  }
  uint32_t dayOfMonth;
  ReadingStatsAnalytics::civilFromDays(day, year, month, dayOfMonth);
}

void ReadingHeatmapActivity::shiftMonth(const int delta) {
  if (month == 0) return;
  const int32_t index = year * 12 + static_cast<int32_t>(month) - 1 + delta;
  year = index / 12;
  month = static_cast<uint32_t>(index % 12) + 1;
  requestUpdate();
}

void ReadingHeatmapActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Left) ||
      mappedInput.wasReleased(MappedInputManager::Button::Up)) {
    shiftMonth(-1);
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Right) ||
             mappedInput.wasReleased(MappedInputManager::Button::Down)) {
    shiftMonth(1);
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    showMonthOf(anchorDay());
    requestUpdate();
  }
}

void ReadingHeatmapActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int pageWidth = renderer.getScreenWidth();
  const int pageHeight = renderer.getScreenHeight();

  renderer.clearScreen();

  char title[40];
  if (month == 0) {
    snprintf(title, sizeof(title), "%s", tr(STR_STATS_CALENDAR));
  } else {
    snprintf(title, sizeof(title), "%s %d", ReadingStatsFormat::monthName(month), static_cast<int>(year));
  }
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, title);

  const int top = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int bottom = pageHeight - metrics.buttonHintsHeight - metrics.verticalSpacing;
  if (month == 0) {
    UITheme::drawCenteredWrappedText(
        renderer, Rect{metrics.contentSidePadding, top, pageWidth - 2 * metrics.contentSidePadding, bottom - top},
        UI_10_FONT_ID, tr(STR_STATS_CLOCK_UNKNOWN), 4);
  } else {
    drawCalendar(metrics, top, bottom);
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_TODAY), tr(STR_PREV), tr(STR_NEXT));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}

void ReadingHeatmapActivity::drawCalendar(const ThemeMetrics& metrics, int y, const int bottom) const {
  const int left = metrics.contentSidePadding;
  const int contentWidth = renderer.getScreenWidth() - 2 * metrics.contentSidePadding;
  const int smallHeight = renderer.getLineHeight(SMALL_FONT_ID);
  const int textHeight = renderer.getLineHeight(UI_10_FONT_ID);
  const auto& days = READING_STATS.getDays();
  const uint32_t goal = READING_STATS.getDailyGoalMinutes();
  const int32_t today = ReadingStatsStore::currentLocalDay();

  const int32_t firstDay = ReadingStatsAnalytics::daysFromCivil(year, month, 1);
  const int monthLength = static_cast<int>(ReadingStatsAnalytics::daysInMonth(year, month));
  const int offset = static_cast<int>(ReadingStatsAnalytics::weekdayMondayFirst(firstDay));
  const int rows = (offset + monthLength + 6) / 7;

  // Month summary: total time and number of reading days.
  uint32_t monthMinutes = 0;
  uint32_t readingDays = 0;
  for (const auto& entry : days) {
    if (entry.day >= firstDay && entry.day < firstDay + monthLength && entry.minutes > 0) {
      monthMinutes += entry.minutes;
      readingDays++;
    }
  }
  char duration[24], summary[64];
  ReadingStatsFormat::formatDuration(duration, sizeof(duration), monthMinutes);
  snprintf(summary, sizeof(summary), tr(STR_STATS_MONTH_SUMMARY_FORMAT), duration, static_cast<unsigned>(readingDays));
  renderer.drawText(UI_10_FONT_ID, left, y, summary, true);
  y += textHeight + SECTION_GAP;

  // Square cells sized to fit both the width and the height left above the legend.
  const int legendHeight = std::max(LEGEND_SWATCH, smallHeight);
  const int gridTop = y + smallHeight + CELL_GAP;
  const int availableHeight = bottom - legendHeight - SECTION_GAP - gridTop;
  const int cellByWidth = (contentWidth - 6 * CELL_GAP) / 7;
  const int cellByHeight = (availableHeight - (MAX_WEEK_ROWS - 1) * CELL_GAP) / MAX_WEEK_ROWS;
  const int cell = std::max(smallHeight + 2 * CELL_TEXT_INSET, std::min(cellByWidth, cellByHeight));
  const int gridLeft = left + (contentWidth - (7 * cell + 6 * CELL_GAP)) / 2;

  for (int col = 0; col < 7; col++) {
    const char* label = ReadingStatsFormat::weekdayShort(col);
    const int labelX = gridLeft + col * (cell + CELL_GAP) + (cell - renderer.getTextWidth(SMALL_FONT_ID, label)) / 2;
    renderer.drawText(SMALL_FONT_ID, labelX, y, label, true);
  }

  for (int i = 0; i < monthLength; i++) {
    const int position = offset + i;
    const int x = gridLeft + (position % 7) * (cell + CELL_GAP);
    const int cellY = gridTop + (position / 7) * (cell + CELL_GAP);
    const int32_t day = firstDay + i;
    const uint8_t level = ReadingStatsAnalytics::getHeatLevel(ReadingStatsAnalytics::getMinutesOnDay(days, day), goal);

    ReadingStatsFormat::fillHeat(renderer, x, cellY, cell, cell, level);
    renderer.drawRect(x, cellY, cell, cell, true);
    if (day == today) {
      renderer.drawRect(x + 2, cellY + 2, cell - 4, cell - 4, 2, level < 2);  // white ring on dark cells
    }

    char number[4];
    snprintf(number, sizeof(number), "%d", i + 1);
    // Dark-gray dither swallows black text, so give the number a white backing;
    // solid black cells (goal met) get white text instead.
    if (level == 2) {
      const int numberWidth = renderer.getTextWidth(SMALL_FONT_ID, number);
      renderer.fillRect(x + CELL_TEXT_INSET - 1, cellY + CELL_TEXT_INSET, numberWidth + 2, smallHeight, false);
    }
    renderer.drawText(SMALL_FONT_ID, x + CELL_TEXT_INSET, cellY + CELL_TEXT_INSET, number, level < 3);
  }

  // Legend
  const int legendY = gridTop + rows * (cell + CELL_GAP) - CELL_GAP + SECTION_GAP;
  const char* legendLabels[3] = {tr(STR_STATS_LEGEND_SOME), tr(STR_STATS_LEGEND_HALF), tr(STR_STATS_LEGEND_GOAL)};
  int legendX = gridLeft;
  for (int level = 1; level <= 3; level++) {
    ReadingStatsFormat::fillHeat(renderer, legendX, legendY, LEGEND_SWATCH, LEGEND_SWATCH, static_cast<uint8_t>(level));
    renderer.drawRect(legendX, legendY, LEGEND_SWATCH, LEGEND_SWATCH, true);
    legendX += LEGEND_SWATCH + CELL_TEXT_INSET * 2;
    const char* label = legendLabels[level - 1];
    renderer.drawText(SMALL_FONT_ID, legendX, legendY + (LEGEND_SWATCH - smallHeight) / 2, label, true);
    legendX += renderer.getTextWidth(SMALL_FONT_ID, label) + LEGEND_ITEM_GAP;
  }
}
