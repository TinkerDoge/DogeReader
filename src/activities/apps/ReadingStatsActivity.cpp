#include "ReadingStatsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>

#include "ReadingHeatmapActivity.h"
#include "ReadingStatsFormat.h"
#include "ReadingStatsStore.h"
#include "RecentBooksStore.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr int TILE_GAP = 8;
constexpr int TILE_PADDING = 12;
constexpr int MAX_TILE_RADIUS = 8;
constexpr int SECTION_GAP = 20;
constexpr int LABEL_GAP = 6;
constexpr int CHART_HEIGHT = 96;
constexpr int GOAL_DASH = 4;
constexpr int BOOK_BAR_HEIGHT = 4;
constexpr int BOOK_ROW_GAP = 14;
}  // namespace

void ReadingStatsActivity::onEnter() {
  Activity::onEnter();
  loadBookRows();
  requestUpdate();
}

void ReadingStatsActivity::loadBookRows() {
  bookRows.clear();
  const auto& recent = RECENT_BOOKS.getBooks();
  bookRows.reserve(recent.size());
  for (const auto& book : recent) {
    const auto* stats = READING_STATS.findBook(book.path.c_str());
    if (stats == nullptr || (stats->minutes == 0 && stats->percent == 0)) continue;
    bookRows.push_back({book.title, stats->minutes, stats->percent});
  }
}

void ReadingStatsActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    activityManager.pushActivity(std::make_unique<ReadingHeatmapActivity>(renderer, mappedInput));
  }
}

void ReadingStatsActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int pageWidth = renderer.getScreenWidth();
  const int pageHeight = renderer.getScreenHeight();

  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_READING_STATS));

  int y = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  y = drawSummaryTiles(metrics, y) + SECTION_GAP;
  y = drawWeekChart(metrics, y) + SECTION_GAP;
  drawBookRows(metrics, y, pageHeight - metrics.buttonHintsHeight - metrics.verticalSpacing);

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_STATS_CALENDAR), "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}

int ReadingStatsActivity::drawSummaryTiles(const ThemeMetrics& metrics, const int y) const {
  const int left = metrics.contentSidePadding;
  const int contentWidth = renderer.getScreenWidth() - 2 * metrics.contentSidePadding;
  const int tileWidth = (contentWidth - TILE_GAP) / 2;
  const int labelHeight = renderer.getLineHeight(SMALL_FONT_ID);
  const int tileHeight = TILE_PADDING * 2 + labelHeight + renderer.getLineHeight(UI_12_FONT_ID);
  const int radius = std::clamp(metrics.popupCornerRadius, 0, MAX_TILE_RADIUS);

  char streak[24], best[24], today[24], total[24];
  ReadingStatsFormat::formatDayCount(streak, sizeof(streak), READING_STATS.getCurrentStreak());
  ReadingStatsFormat::formatDayCount(best, sizeof(best), READING_STATS.getBestStreak());
  snprintf(today, sizeof(today), tr(STR_STATS_TODAY_FORMAT), static_cast<unsigned>(READING_STATS.getTodayMinutes()),
           static_cast<unsigned>(READING_STATS.getDailyGoalMinutes()));
  ReadingStatsFormat::formatDuration(total, sizeof(total), READING_STATS.getLifetimeMinutes());

  const char* labels[4] = {tr(STR_STATS_CURRENT_STREAK), tr(STR_STATS_BEST_STREAK), tr(STR_STATS_TODAY),
                           tr(STR_STATS_TOTAL_TIME)};
  const char* values[4] = {streak, best, today, total};
  const int textWidth = tileWidth - 2 * TILE_PADDING;
  for (int i = 0; i < 4; i++) {
    const int x = left + (i % 2) * (tileWidth + TILE_GAP);
    const int tileY = y + (i / 2) * (tileHeight + TILE_GAP);
    renderer.drawRoundedRect(x, tileY, tileWidth, tileHeight, 1, radius, true);
    const auto label = renderer.truncatedText(SMALL_FONT_ID, labels[i], textWidth);
    renderer.drawText(SMALL_FONT_ID, x + TILE_PADDING, tileY + TILE_PADDING, label.c_str(), true);
    const auto value = renderer.truncatedText(UI_12_FONT_ID, values[i], textWidth, EpdFontFamily::BOLD);
    renderer.drawText(UI_12_FONT_ID, x + TILE_PADDING, tileY + TILE_PADDING + labelHeight, value.c_str(), true,
                      EpdFontFamily::BOLD);
  }
  return y + 2 * tileHeight + TILE_GAP;
}

int ReadingStatsActivity::drawWeekChart(const ThemeMetrics& metrics, int y) const {
  const int left = metrics.contentSidePadding;
  const int contentWidth = renderer.getScreenWidth() - 2 * metrics.contentSidePadding;
  const int titleHeight = renderer.getLineHeight(UI_10_FONT_ID);
  const int smallHeight = renderer.getLineHeight(SMALL_FONT_ID);
  const uint32_t goal = std::max<uint32_t>(READING_STATS.getDailyGoalMinutes(), 1);

  renderer.drawText(UI_10_FONT_ID, left, y, tr(STR_STATS_LAST_7_DAYS), true, EpdFontFamily::BOLD);
  char goalLabel[24];
  snprintf(goalLabel, sizeof(goalLabel), tr(STR_STATS_GOAL_FORMAT), static_cast<unsigned>(goal));
  renderer.drawText(SMALL_FONT_ID, left + contentWidth - renderer.getTextWidth(SMALL_FONT_ID, goalLabel),
                    y + (titleHeight - smallHeight) / 2, goalLabel, true);
  y += titleHeight + LABEL_GAP;

  const int32_t today = ReadingStatsStore::currentLocalDay();
  if (today < 0) {
    UITheme::drawCenteredWrappedText(renderer, Rect{left, y, contentWidth, CHART_HEIGHT + LABEL_GAP + smallHeight},
                                     SMALL_FONT_ID, tr(STR_STATS_CLOCK_UNKNOWN), 3);
    return y + CHART_HEIGHT + LABEL_GAP + smallHeight;
  }

  const auto& days = READING_STATS.getDays();
  uint32_t minutes[7];
  uint32_t scaleMax = goal;
  for (int i = 0; i < 7; i++) {
    minutes[i] = ReadingStatsAnalytics::getMinutesOnDay(days, today - 6 + i);
    scaleMax = std::max(scaleMax, minutes[i]);
  }

  const int chartBottom = y + CHART_HEIGHT;
  const int slotWidth = contentWidth / 7;
  const int barWidth = slotWidth * 3 / 5;
  for (int i = 0; i < 7; i++) {
    const int slotX = left + i * slotWidth;
    if (minutes[i] > 0) {
      const int barX = slotX + (slotWidth - barWidth) / 2;
      const int barHeight = std::max(2, static_cast<int>(minutes[i] * CHART_HEIGHT / scaleMax));
      const uint8_t level = ReadingStatsAnalytics::getHeatLevel(minutes[i], goal);
      ReadingStatsFormat::fillHeat(renderer, barX, chartBottom - barHeight, barWidth, barHeight, level);
      renderer.drawRect(barX, chartBottom - barHeight, barWidth, barHeight, true);
    }

    const char* label = ReadingStatsFormat::weekdayShort(ReadingStatsAnalytics::weekdayMondayFirst(today - 6 + i));
    const int labelWidth = renderer.getTextWidth(SMALL_FONT_ID, label);
    const int labelX = slotX + (slotWidth - labelWidth) / 2;
    const int labelY = chartBottom + LABEL_GAP;
    renderer.drawText(SMALL_FONT_ID, labelX, labelY, label, true);
    if (i == 6) {  // underline today
      renderer.drawLine(labelX, labelY + smallHeight, labelX + labelWidth - 1, labelY + smallHeight, 2, true);
    }
  }

  // Baseline, then a dashed goal line drawn over the bars.
  renderer.drawLine(left, chartBottom, left + contentWidth - 1, chartBottom, true);
  const int goalY = chartBottom - static_cast<int>(goal * CHART_HEIGHT / scaleMax);
  for (int x = left; x < left + contentWidth; x += GOAL_DASH * 2) {
    renderer.drawLine(x, goalY, std::min(x + GOAL_DASH, left + contentWidth) - 1, goalY, true);
  }
  return chartBottom + LABEL_GAP + smallHeight;
}

void ReadingStatsActivity::drawBookRows(const ThemeMetrics& metrics, int y, const int bottom) const {
  const int left = metrics.contentSidePadding;
  const int contentWidth = renderer.getScreenWidth() - 2 * metrics.contentSidePadding;
  const int titleHeight = renderer.getLineHeight(UI_10_FONT_ID);
  const int smallHeight = renderer.getLineHeight(SMALL_FONT_ID);

  renderer.drawText(UI_10_FONT_ID, left, y, tr(STR_STATS_BOOKS), true, EpdFontFamily::BOLD);
  y += titleHeight + LABEL_GAP;

  if (bookRows.empty()) {
    const auto hint = renderer.truncatedText(SMALL_FONT_ID, tr(STR_STATS_NO_BOOKS), contentWidth);
    renderer.drawText(SMALL_FONT_ID, left, y, hint.c_str(), true);
    return;
  }

  const int rowHeight = titleHeight + LABEL_GAP + BOOK_BAR_HEIGHT;
  for (const auto& row : bookRows) {
    if (y + rowHeight > bottom) break;

    char duration[24], detail[40];
    ReadingStatsFormat::formatDuration(duration, sizeof(duration), row.minutes);
    snprintf(detail, sizeof(detail), "%s  %u%%", duration, static_cast<unsigned>(row.percent));
    const int detailWidth = renderer.getTextWidth(SMALL_FONT_ID, detail);
    renderer.drawText(SMALL_FONT_ID, left + contentWidth - detailWidth, y + (titleHeight - smallHeight) / 2, detail,
                      true);
    const auto title = renderer.truncatedText(UI_10_FONT_ID, row.title.c_str(), contentWidth - detailWidth - TILE_GAP);
    renderer.drawText(UI_10_FONT_ID, left, y, title.c_str(), true);

    const int barY = y + titleHeight + LABEL_GAP;
    renderer.fillRectDither(left, barY, contentWidth, BOOK_BAR_HEIGHT, Color::LightGray);
    renderer.fillRect(left, barY, contentWidth * row.percent / 100, BOOK_BAR_HEIGHT, true);
    y += rowHeight + BOOK_ROW_GAP;
  }
}
