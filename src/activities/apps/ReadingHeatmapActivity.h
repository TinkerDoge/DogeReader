#pragma once
#include "../Activity.h"
#include "../../util/ButtonNavigator.h"

class ReadingHeatmapActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  bool firstRenderDone = false;
  int currentMonth;
  int currentYear;

 public:
  explicit ReadingHeatmapActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ReadingHeatmap", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  void drawGrid();
  void drawLegend();
};
