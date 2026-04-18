#include "ReadingHeatmapActivity.h"
#include <GfxRenderer.h>
#include <I18n.h>
#include <Logging.h>
#include "components/UITheme.h"
#include "../../util/ReadingStatsAnalytics.h"
#include "ReadingStatsActivity.h"
#include "../../fontIds.h"

void ReadingHeatmapActivity::onEnter() {
  Activity::onEnter();
  READING_STATS.loadFromFile();
  
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);
  currentMonth = t->tm_mon + 1;
  currentYear = t->tm_year + 1900;
  
  requestUpdate();
}

void ReadingHeatmapActivity::onExit() {
  Activity::onExit();
}

void ReadingHeatmapActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    activityManager.popActivity();
  }
  
  if (mappedInput.wasReleased(MappedInputManager::Button::Left)) {
    if (--currentMonth == 0) {
      currentMonth = 12;
      currentYear--;
    }
    requestUpdate();
  }
  
  if (mappedInput.wasReleased(MappedInputManager::Button::Right)) {
    if (++currentMonth == 13) {
      currentMonth = 1;
      currentYear++;
    }
    requestUpdate();
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    // Navigate back to Stats
    activityManager.pushActivity(std::make_unique<ReadingStatsActivity>(renderer, mappedInput));
  }
}

void ReadingHeatmapActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  renderer.clearScreen();

  char titleBuf[32];
  snprintf(titleBuf, sizeof(titleBuf), "%02d/%04d", currentMonth, currentYear);
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, titleBuf);

  drawGrid();
  drawLegend();

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "Stats", tr(STR_PREV), tr(STR_NEXT));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer(firstRenderDone ? HalDisplay::FAST_REFRESH : HalDisplay::FULL_REFRESH);
  firstRenderDone = true;
}

void ReadingHeatmapActivity::drawGrid() {
  auto monthStats = READING_STATS.getStatsForMonth(currentMonth, currentYear);
  std::map<int, uint32_t> dayMins;
  uint32_t monthTotal = 0;
  uint32_t daysRead = 0;
  uint32_t bestDay = 0;

  for (const auto& day : monthStats) {
    int d = atoi(day.date.substr(8, 2).c_str());
    dayMins[d] = day.totalMinutes;
    monthTotal += day.totalMinutes;
    if (day.totalMinutes > 0) daysRead++;
    if (day.totalMinutes > bestDay) bestDay = day.totalMinutes;
  }

  // Draw Header Stats
  const int statsY = 100;
  char buf1[64], buf2[64];
  snprintf(buf1, sizeof(buf1), "Total: %dh %dm", monthTotal / 60, monthTotal % 60);
  snprintf(buf2, sizeof(buf2), "Read: %d days", daysRead);
  renderer.drawText(UI_10_FONT_ID, 20, statsY, buf1, true);
  renderer.drawText(UI_10_FONT_ID, 240, statsY, buf2, true);

  // Calendar Logic
  struct tm firstDay = {0};
  firstDay.tm_mday = 1;
  firstDay.tm_mon = currentMonth - 1;
  firstDay.tm_year = currentYear - 1900;
  firstDay.tm_isdst = -1;
  mktime(&firstDay);
  
  int startDayOfWeek = (firstDay.tm_wday + 6) % 7; // Adjust to Monday start
  int daysInMonth = 31;
  if (currentMonth == 4 || currentMonth == 6 || currentMonth == 9 || currentMonth == 11) daysInMonth = 30;
  else if (currentMonth == 2) {
    daysInMonth = (currentYear % 4 == 0 && (currentYear % 100 != 0 || currentYear % 400 == 0)) ? 29 : 28;
  }

  const int cellSize = 58;
  const int spacing = 6;
  const int startX = 20;
  const int startY = 160;

  // Day Labels
  const char* dayLabels[] = {"M", "T", "W", "T", "F", "S", "S"};
  for (int i = 0; i < 7; i++) {
    renderer.drawText(UI_10_FONT_ID, startX + i * (cellSize + spacing) + 20, startY - 30, dayLabels[i], true);
  }

  for (int i = 0; i < daysInMonth; i++) {
    int dayNum = i + 1;
    int pos = i + startDayOfWeek;
    int col = pos % 7;
    int row = pos / 7;
    
    int x = startX + col * (cellSize + spacing);
    int y = startY + row * (cellSize + spacing);

    uint32_t mins = dayMins[dayNum];
    uint8_t heat = ReadingStatsAnalytics::getHeatLevel(mins);

    Color color = White;
    if (heat == 1) color = LightGray;
    else if (heat == 2) color = static_cast<Color>(0x08);
    else if (heat == 3) color = DarkGray;
    else if (heat == 4) color = Black;

    if (color == Black) {
      renderer.fillRect(x, y, cellSize, cellSize, true);
    } else {
      if (heat > 0) renderer.fillRectDither(x, y, cellSize, cellSize, color);
      renderer.drawRect(x, y, cellSize, cellSize, true);
    }
    
    char dayBuf[4];
    snprintf(dayBuf, sizeof(dayBuf), "%d", dayNum);
    renderer.drawText(UI_10_FONT_ID, x + 5, y + 5, dayBuf, color == Black ? false : true);
  }
}

void ReadingHeatmapActivity::drawLegend() {
  const int legendY = 660; // Slightly lower
  const int startX = 40;
  const int boxSize = 24;
  const int spacing = 105;

  auto drawLegendItem = [&](int x, Color color, const char* label) {
    if (color == Black) {
      renderer.fillRect(x, legendY, boxSize, boxSize, true);
    } else if (color != White) {
      renderer.fillRectDither(x, legendY, boxSize, boxSize, color);
    }
    renderer.drawRect(x, legendY, boxSize, boxSize, true);
    renderer.drawText(UI_10_FONT_ID, x + boxSize + 10, legendY + 5, label, true);
  };

  drawLegendItem(startX, LightGray, "1m+");
  drawLegendItem(startX + spacing, static_cast<Color>(0x08), "10m+");
  drawLegendItem(startX + 2 * spacing, DarkGray, "30m+");
  drawLegendItem(startX + 3 * spacing, Black, "60m+");
}
