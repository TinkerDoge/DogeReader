#pragma once
#include <vector>
#include <string>
#include "../Activity.h"
#include "util/ButtonNavigator.h"

class ReadingStatsActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  int selectorIndex = 0;
  bool firstRenderDone = false;
  
  struct BookStat {
    std::string path;
    std::string title;
    uint32_t totalMinutes;
    int progressPercent;
  };
  std::vector<BookStat> startedBooks;

  void loadStats();
  void drawStatsHeader();
  void drawBookList();

 public:
  explicit ReadingStatsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ReadingStats", renderer, mappedInput) {}
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};
