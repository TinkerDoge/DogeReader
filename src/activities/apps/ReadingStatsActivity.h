#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "../Activity.h"

struct ThemeMetrics;

// Summary tiles, a 7-day bar chart and per-book time for recently read books.
// Confirm opens the month calendar.
class ReadingStatsActivity final : public Activity {
  struct BookRow {
    std::string title;
    uint32_t minutes;
    uint8_t percent;
  };
  std::vector<BookRow> bookRows;
  bool firstRenderDone = false;

  void loadBookRows();
  int drawSummaryTiles(const ThemeMetrics& metrics, int y) const;
  int drawWeekChart(const ThemeMetrics& metrics, int y) const;
  void drawBookRows(const ThemeMetrics& metrics, int y, int bottom) const;

 public:
  explicit ReadingStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ReadingStats", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
