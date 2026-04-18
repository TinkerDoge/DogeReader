#include "AppsActivity.h"
#include <GfxRenderer.h>
#include <I18n.h>
#include "../ActivityManager.h"
#include "../tarot/TarotActivity.h"
#include "../apps/ReadingHeatmapActivity.h"
#include "../apps/ReadingStatsActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"

void AppsActivity::onEnter() {
  Activity::onEnter();
  selectorIndex = 0;
  requestUpdate();
}

void AppsActivity::onExit() {
  Activity::onExit();
}

void AppsActivity::loop() {
  const int appCount = 2;

  buttonNavigator.onNext([this, appCount] {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, appCount);
    requestUpdate();
  });

  buttonNavigator.onPrevious([this, appCount] {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, appCount);
    requestUpdate();
  });

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    if (selectorIndex == 0) {
      onTarotOpen();
    } else if (selectorIndex == 1) {
      onReadingStatsOpen();
    }
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    activityManager.popActivity();
  }
}

void AppsActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  renderer.clearScreen();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_APPS_TITLE));

  std::vector<const char*> appNames = {tr(STR_TAROT_TITLE), tr(STR_READING_STATS)};
  std::vector<const char*> appDescs = {tr(STR_TAROT_DESC), tr(STR_READING_STATS_DESC)};
  std::vector<UIIcon> appIcons = {Tarot, Heatmap};

  GUI.drawList(
      renderer,
      Rect{0, metrics.headerHeight + metrics.verticalSpacing, pageWidth,
           pageHeight - (metrics.headerHeight + metrics.verticalSpacing + metrics.buttonHintsHeight)},
      static_cast<int>(appNames.size()), selectorIndex,
      [&appNames](int index) { return std::string(appNames[index]); },
      [&appDescs](int index) { return std::string(appDescs[index]); },
      [&appIcons](int index) { return appIcons[index]; });

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer(firstRenderDone ? HalDisplay::FAST_REFRESH : HalDisplay::FULL_REFRESH);
  firstRenderDone = true;
}

void AppsActivity::onTarotOpen() {
  activityManager.pushActivity(std::make_unique<TarotActivity>(renderer, mappedInput));
}

void AppsActivity::onReadingStatsOpen() {
  activityManager.pushActivity(std::make_unique<ReadingStatsActivity>(renderer, mappedInput));
}

void AppsActivity::onReadingHeatmapOpen() {
  activityManager.pushActivity(std::make_unique<ReadingHeatmapActivity>(renderer, mappedInput));
}
