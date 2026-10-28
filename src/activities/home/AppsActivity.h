#pragma once
#include "../Activity.h"
#include "util/ButtonNavigator.h"

class AppsActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  int selectorIndex = 0;

  void openSelected();

 public:
  explicit AppsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Apps", renderer, mappedInput) {}
  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
