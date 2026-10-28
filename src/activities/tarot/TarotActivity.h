#pragma once

#include <cstdint>
#include <vector>

#include "../Activity.h"
#include "TarotAssets.h"
#include "TarotDeck.h"

struct ThemeMetrics;

class TarotActivity final : public Activity {
 public:
  TarotActivity(GfxRenderer& renderer, MappedInputManager& mappedInput) : Activity("Tarot", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&& lock) override;

 private:
  enum class ViewState { DrawPrompt, Main, HistoryGrid };

  TarotDeck deck;
  TarotAssets assets;
  ViewState viewState = ViewState::DrawPrompt;
  bool assetsAvailable = false;
  bool showMeaning = false;
  bool reshuffled = false;
  int8_t currentCardId = -1;
  std::vector<int8_t> history;  // most recent first, at most one full deck
  int gridPageIndex = 0;

  void drawNextCard();
  int gridItemsPerPage(const ThemeMetrics& metrics) const;
  void renderPrompt(const ThemeMetrics& metrics) const;
  void renderMain(const ThemeMetrics& metrics) const;
  void renderMeaning(const ThemeMetrics& metrics) const;
  void renderGrid(const ThemeMetrics& metrics) const;
  void renderMissingAssets(const ThemeMetrics& metrics) const;
};
