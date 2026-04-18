#include "ReadingProfileActivity.h"
#include <GfxRenderer.h>
#include <I18n.h>
#include <cmath>
#include <set>
#include "components/UITheme.h"
#include "../../ReadingStatsStore.h"
#include "../../fontIds.h"

void ReadingProfileActivity::onEnter() {
  Activity::onEnter();
  calculateMetrics();
  requestUpdate();
}

void ReadingProfileActivity::onExit() {
  Activity::onExit();
}

void ReadingProfileActivity::calculateMetrics() {
  auto history = READING_STATS.getHistory();
  if (history.empty()) {
    metrics = {0.2, 0.2, 0.2, 0.2, 0.2};
    return;
  }

  // Simplified metric calculation
  metrics.frequency = std::min(1.0f, (float)history.size() / 30.0f);
  
  uint32_t totalMins = READING_STATS.getLifetimeMinutes();
  metrics.duration = std::min(1.0f, (float)totalMins / 10000.0f);

  // Stability (streak vs total days)
  metrics.stability = std::min(1.0f, (float)READING_STATS.getCurrentStreak() / 14.0f);

  // Intensity (mins per day)
  float avgMins = (float)totalMins / history.size();
  metrics.intensity = std::min(1.0f, avgMins / 60.0f);

  // Variety (books read)
  std::set<std::string> books;
  for (const auto& day : history) {
    for (const auto& [path, mins] : day.bookMinutes) {
      books.insert(path);
    }
  }
  metrics.variety = std::min(1.0f, (float)books.size() / 10.0f);
  
  // Ensure minimum for visibility
  metrics.stability = std::max(0.1f, metrics.stability);
  metrics.frequency = std::max(0.1f, metrics.frequency);
  metrics.intensity = std::max(0.1f, metrics.intensity);
  metrics.variety = std::max(0.1f, metrics.variety);
  metrics.duration = std::max(0.1f, metrics.duration);
}

void ReadingProfileActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    activityManager.popActivity();
  }
}

void ReadingProfileActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, renderer.getScreenWidth(), metrics.headerHeight}, "Reading Profile");

  drawRadarChart(renderer.getScreenWidth() / 2, 400, 180);

  renderer.drawText(UI_10_FONT_ID, 20, 650, "Your reading habits are well-balanced.", true);

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer(firstRenderDone ? HalDisplay::FAST_REFRESH : HalDisplay::FULL_REFRESH);
  firstRenderDone = true;
}

void ReadingProfileActivity::drawRadarChart(int centerX, int centerY, int radius) {
  const int numAxes = 5;
  const char* labels[] = {"Stability", "Frequency", "Intensity", "Variety", "Duration"};
  float values[] = {this->metrics.stability, this->metrics.frequency, this->metrics.intensity, this->metrics.variety, this->metrics.duration};

  int xPoints[numAxes];
  int yPoints[numAxes];

  for (int i = 0; i < numAxes; i++) {
    float angle = (i * 2 * 3.14159f / numAxes) - (3.14159f / 2.0f);
    
    // Outer grid (pentagons)
    for (int level = 1; level <= 4; level++) {
      float r = radius * (level / 4.0f);
      float nextAngle = ((i + 1) * 2 * 3.14159f / numAxes) - (3.14159f / 2.0f);
      renderer.drawLine(centerX + r * cos(angle), centerY + r * sin(angle),
                        centerX + r * cos(nextAngle), centerY + r * sin(nextAngle));
    }

    // Axes
    int ox = centerX + radius * cos(angle);
    int oy = centerY + radius * sin(angle);
    renderer.drawLine(centerX, centerY, ox, oy);
    
    // Labels
    int lx = centerX + (radius + 40) * cos(angle) - 30;
    int ly = centerY + (radius + 20) * sin(angle);
    renderer.drawText(SMALL_FONT_ID, lx, ly, labels[i], true);

    // Data points
    xPoints[i] = centerX + (radius * values[i]) * cos(angle);
    yPoints[i] = centerY + (radius * values[i]) * sin(angle);
  }

  // Draw data polygon
  for (int i = 0; i < numAxes; i++) {
    renderer.drawLine(xPoints[i], yPoints[i], xPoints[(i + 1) % numAxes], yPoints[(i + 1) % numAxes], 3);
  }
}
