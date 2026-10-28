#pragma once
#include <cstdint>

#include "../Activity.h"

struct ThemeMetrics;

// Month calendar shaded by reading time relative to the daily goal.
// Left/Right (or Up/Down) change month, Confirm jumps back to today.
class ReadingHeatmapActivity final : public Activity {
  int32_t year = 0;
  uint32_t month = 0;  // 1-12; 0 when there is no dated reading to show

  void showMonthOf(int32_t day);
  void shiftMonth(int delta);
  void drawCalendar(const ThemeMetrics& metrics, int y, int bottom) const;

 public:
  explicit ReadingHeatmapActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ReadingHeatmap", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
