#pragma once
#include <vector>
#include <string>
#include "../Activity.h"

class ReadingProfileActivity final : public Activity {
  bool firstRenderDone = false;
  
  struct HabitMetrics {
    float stability;
    float frequency;
    float intensity;
    float variety;
    float duration;
  };
  HabitMetrics metrics;

  void calculateMetrics();
  void drawRadarChart(int centerX, int centerY, int radius);

 public:
  explicit ReadingProfileActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ReadingProfile", renderer, mappedInput) {}
  void onEnter() override;
  void onExit() override;
  void render(RenderLock&&) override;
  void loop() override;
};
