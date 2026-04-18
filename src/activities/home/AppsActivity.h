#pragma once
#include <vector>
#include <string>
#include "../Activity.h"
#include "util/ButtonNavigator.h"

class AppsActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  int selectorIndex = 0;
  bool firstRenderDone = false;
  
  void onTarotOpen();
  void onReadingStatsOpen();
  void onReadingHeatmapOpen();

 public:
  explicit AppsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Apps", renderer, mappedInput) {}
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};
