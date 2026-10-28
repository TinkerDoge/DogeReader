#include "AppsActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "../ActivityManager.h"
#include "../apps/ReadingStatsActivity.h"
#include "../tarot/TarotActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
enum class AppId { ReadingStats, Tarot };

struct AppEntry {
  AppId id;
  StrId title;
  StrId description;
  UIIcon icon;
};

constexpr AppEntry APPS[] = {
    {AppId::ReadingStats, StrId::STR_READING_STATS, StrId::STR_READING_STATS_DESC, UIIcon::Heatmap},
    {AppId::Tarot, StrId::STR_TAROT_TITLE, StrId::STR_TAROT_DESC, UIIcon::Tarot},
};
constexpr int APP_COUNT = sizeof(APPS) / sizeof(APPS[0]);
}  // namespace

void AppsActivity::onEnter() {
  Activity::onEnter();
  selectorIndex = 0;
  requestUpdate();
}

void AppsActivity::loop() {
  buttonNavigator.onNext([this] {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, APP_COUNT);
    requestUpdate();
  });

  buttonNavigator.onPrevious([this] {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, APP_COUNT);
    requestUpdate();
  });

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    openSelected();
  } else if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    // Apps replaced Home, so return there with the Apps entry selected.
    activityManager.goHome(HomeMenuItem::APPS);
  }
}

void AppsActivity::openSelected() {
  switch (APPS[selectorIndex].id) {
    case AppId::ReadingStats:
      activityManager.pushActivity(std::make_unique<ReadingStatsActivity>(renderer, mappedInput));
      break;
    case AppId::Tarot:
      activityManager.pushActivity(std::make_unique<TarotActivity>(renderer, mappedInput));
      break;
  }
}

void AppsActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_APPS_TITLE));

  const int listTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const auto& i18n = I18n::getInstance();
  GUI.drawList(
      renderer, Rect{0, listTop, pageWidth, pageHeight - listTop - metrics.buttonHintsHeight - metrics.verticalSpacing},
      APP_COUNT, selectorIndex, [&i18n](int index) { return std::string(i18n.get(APPS[index].title)); },
      [&i18n](int index) { return std::string(i18n.get(APPS[index].description)); },
      [](int index) { return APPS[index].icon; });

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
