#include "TarotActivity.h"

#include <Bitmap.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <I18n.h>

#include <algorithm>
#include <string>

#include "components/UITheme.h"
#include "fontIds.h"

namespace {
// Native asset sizes from the README's Tarot guide; images are only ever scaled down.
constexpr int THUMB_WIDTH = 80;
constexpr int THUMB_HEIGHT = 133;
constexpr int THUMB_ROW_COUNT = 5;
constexpr int THUMB_GAP = 12;
constexpr int SECTION_GAP = 12;
constexpr int CAPTION_LINE_GAP = 4;
constexpr int CARD_FRAME = 2;
constexpr int GRID_COLUMNS = 4;
constexpr int GRID_GAP_X = 20;
constexpr int GRID_GAP_Y = 12;
constexpr int MEANING_PADDING = 24;
constexpr int MEANING_TITLE_GAP = 12;
constexpr int MEANING_MAX_LINES = 9;
constexpr int MEANING_RADIUS = 8;

// Draws a BMP centered in `box`, scaled down to fit. Returns the drawn rect
// (zero width if the file is missing or unreadable).
Rect drawBmpCentered(const GfxRenderer& renderer, const std::string& path, const Rect& box) {
  HalFile file;
  if (!Storage.openFileForRead("TAROT", path, file)) return Rect{};
  Bitmap bitmap(file, true);
  if (bitmap.parseHeaders() != BmpReaderError::Ok || bitmap.getWidth() <= 0 || bitmap.getHeight() <= 0) {
    return Rect{};
  }
  const float scale = std::min({1.0f, static_cast<float>(box.width) / static_cast<float>(bitmap.getWidth()),
                                static_cast<float>(box.height) / static_cast<float>(bitmap.getHeight())});
  const int width = static_cast<int>(static_cast<float>(bitmap.getWidth()) * scale);
  const int height = static_cast<int>(static_cast<float>(bitmap.getHeight()) * scale);
  const Rect drawn{box.x + (box.width - width) / 2, box.y + (box.height - height) / 2, width, height};
  renderer.drawBitmap(bitmap, drawn.x, drawn.y, drawn.width, drawn.height);
  return drawn;
}

int contentBottom(const GfxRenderer& renderer, const ThemeMetrics& metrics) {
  return renderer.getScreenHeight() - metrics.buttonHintsHeight - metrics.verticalSpacing;
}
}  // namespace

void TarotActivity::onEnter() {
  Activity::onEnter();
  assetsAvailable = assets.loadMeanings();
  deck.shuffle();
  history.clear();
  history.reserve(TarotDeck::DECK_SIZE);
  currentCardId = -1;
  showMeaning = false;
  reshuffled = false;
  viewState = ViewState::DrawPrompt;
  requestUpdate();
}

void TarotActivity::loop() {
  Activity::loop();

  const bool back = mappedInput.wasReleased(MappedInputManager::Button::Back);
  const bool confirm = mappedInput.wasReleased(MappedInputManager::Button::Confirm);
  const bool left = mappedInput.wasReleased(MappedInputManager::Button::Left);
  const bool right = mappedInput.wasReleased(MappedInputManager::Button::Right);

  if (!assetsAvailable) {
    if (back) finish();
    return;
  }

  switch (viewState) {
    case ViewState::DrawPrompt:
      if (right) {
        drawNextCard();
      } else if (back) {
        finish();
      }
      break;

    case ViewState::Main:
      if (right) {
        drawNextCard();
      } else if (left) {
        showMeaning = !showMeaning;
        requestUpdate();
      } else if (confirm && history.size() > 1) {
        viewState = ViewState::HistoryGrid;
        gridPageIndex = 0;
        showMeaning = false;
        requestUpdate();
      } else if (back) {
        if (showMeaning) {
          showMeaning = false;
          requestUpdate();
        } else {
          finish();
        }
      }
      break;

    case ViewState::HistoryGrid: {
      const int perPage = gridItemsPerPage(UITheme::getInstance().getMetrics());
      const int lastPage = static_cast<int>(history.size() - 1) / perPage;
      if (back) {
        viewState = ViewState::Main;
        requestUpdate();
      } else if (left && gridPageIndex > 0) {
        gridPageIndex--;
        requestUpdate();
      } else if (right && gridPageIndex < lastPage) {
        gridPageIndex++;
        requestUpdate();
      }
      break;
    }
  }
}

void TarotActivity::drawNextCard() {
  if (!deck.hasNext()) {
    // Every card has been drawn: start a fresh deck and say so.
    deck.shuffle();
    history.clear();
    currentCardId = -1;
    reshuffled = true;
    viewState = ViewState::DrawPrompt;
    requestUpdate();
    return;
  }

  currentCardId = deck.drawNext();
  history.insert(history.begin(), currentCardId);
  reshuffled = false;
  showMeaning = false;
  viewState = ViewState::Main;
  requestUpdate();
}

void TarotActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  renderer.clearScreen();

  if (!assetsAvailable) {
    renderMissingAssets(metrics);
  } else {
    switch (viewState) {
      case ViewState::DrawPrompt:
        renderPrompt(metrics);
        break;
      case ViewState::Main:
        renderMain(metrics);
        if (showMeaning) renderMeaning(metrics);
        break;
      case ViewState::HistoryGrid:
        renderGrid(metrics);
        break;
    }
  }

  renderer.displayBuffer();
}

void TarotActivity::renderMissingAssets(const ThemeMetrics& metrics) const {
  const int pageWidth = renderer.getScreenWidth();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_TAROT_TITLE));
  const int top = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  UITheme::drawCenteredWrappedText(renderer,
                                   Rect{metrics.contentSidePadding, top, pageWidth - 2 * metrics.contentSidePadding,
                                        contentBottom(renderer, metrics) - top},
                                   UI_10_FONT_ID, tr(STR_TAROT_MISSING_ASSETS), 4);
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void TarotActivity::renderPrompt(const ThemeMetrics& metrics) const {
  const int pageWidth = renderer.getScreenWidth();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_TAROT_TITLE));

  const int top = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int textHeight = renderer.getLineHeight(UI_10_FONT_ID);
  const int textY = contentBottom(renderer, metrics) - textHeight;
  drawBmpCentered(
      renderer, TarotAssets::getBackImagePath(),
      Rect{metrics.contentSidePadding, top, pageWidth - 2 * metrics.contentSidePadding, textY - SECTION_GAP - top});
  renderer.drawCenteredText(UI_10_FONT_ID, textY, reshuffled ? tr(STR_TAROT_RESHUFFLED) : tr(STR_TAROT_PROMPT));

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", tr(STR_TAROT_DRAW));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void TarotActivity::renderMain(const ThemeMetrics& metrics) const {
  const int pageWidth = renderer.getScreenWidth();
  const int contentWidth = pageWidth - 2 * metrics.contentSidePadding;

  // 1. Previously drawn cards, most recent on the left.
  const int thumbY = metrics.topPadding + metrics.verticalSpacing;
  const int rowWidth = THUMB_ROW_COUNT * THUMB_WIDTH + (THUMB_ROW_COUNT - 1) * THUMB_GAP;
  const int rowX = (pageWidth - rowWidth) / 2;
  for (int i = 0; i < THUMB_ROW_COUNT && static_cast<size_t>(i + 1) < history.size(); i++) {
    const Rect thumb{rowX + i * (THUMB_WIDTH + THUMB_GAP), thumbY, THUMB_WIDTH, THUMB_HEIGHT};
    const Rect drawn = drawBmpCentered(renderer, TarotAssets::getCardThumbPath(history[i + 1]), thumb);
    if (drawn.width > 0) renderer.drawRect(drawn.x, drawn.y, drawn.width, drawn.height, true);
  }

  // 2. Caption: card name, then its position in this deck.
  const int nameHeight = renderer.getLineHeight(UI_12_FONT_ID);
  const int smallHeight = renderer.getLineHeight(SMALL_FONT_ID);
  const int captionY = contentBottom(renderer, metrics) - nameHeight - CAPTION_LINE_GAP - smallHeight;
  const auto meaning = assets.getMeaning(currentCardId);
  const auto name = renderer.truncatedText(UI_12_FONT_ID, meaning.name, contentWidth, EpdFontFamily::BOLD);
  renderer.drawCenteredText(UI_12_FONT_ID, captionY, name.c_str(), true, EpdFontFamily::BOLD);
  char position[32];
  snprintf(position, sizeof(position), tr(STR_TAROT_CARD_FORMAT), static_cast<unsigned>(deck.getDrawnCount()),
           static_cast<unsigned>(TarotDeck::DECK_SIZE));
  renderer.drawCenteredText(SMALL_FONT_ID, captionY + nameHeight + CAPTION_LINE_GAP, position);

  // 3. The drawn card fills the space between the two, framed.
  const int cardTop = thumbY + THUMB_HEIGHT + SECTION_GAP;
  const Rect cardBox{metrics.contentSidePadding, cardTop, contentWidth, captionY - SECTION_GAP - cardTop};
  const Rect drawn = drawBmpCentered(renderer, TarotAssets::getCardImagePath(currentCardId), cardBox);
  if (drawn.width > 0) {
    renderer.drawRect(drawn.x - CARD_FRAME, drawn.y - CARD_FRAME, drawn.width + 2 * CARD_FRAME,
                      drawn.height + 2 * CARD_FRAME, CARD_FRAME, true);
  }

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), history.size() > 1 ? tr(STR_TAROT_HISTORY) : "",
                                            tr(STR_TAROT_MEANING), tr(STR_TAROT_DRAW));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void TarotActivity::renderMeaning(const ThemeMetrics& metrics) const {
  const auto meaning = assets.getMeaning(currentCardId);
  const int boxWidth = renderer.getScreenWidth() - 2 * metrics.contentSidePadding;
  const int textWidth = boxWidth - 2 * MEANING_PADDING;
  const auto lines = renderer.wrappedText(NOTOSERIF_14_FONT_ID, meaning.meaning, textWidth, MEANING_MAX_LINES);
  const int lineHeight = renderer.getLineHeight(NOTOSERIF_14_FONT_ID) + 4;
  const int titleHeight = renderer.getLineHeight(UI_12_FONT_ID);
  const int boxHeight =
      2 * MEANING_PADDING + titleHeight + 2 * MEANING_TITLE_GAP + static_cast<int>(lines.size()) * lineHeight;
  const int boxX = metrics.contentSidePadding;
  const int boxY = (renderer.getScreenHeight() - boxHeight) / 2;

  renderer.fillRoundedRect(boxX - 2, boxY - 2, boxWidth + 4, boxHeight + 4, MEANING_RADIUS + 2, Color::Black);
  renderer.fillRoundedRect(boxX, boxY, boxWidth, boxHeight, MEANING_RADIUS, Color::White);

  int y = boxY + MEANING_PADDING;
  const auto title = renderer.truncatedText(UI_12_FONT_ID, meaning.name, textWidth, EpdFontFamily::BOLD);
  renderer.drawCenteredText(UI_12_FONT_ID, y, title.c_str(), true, EpdFontFamily::BOLD);
  y += titleHeight + MEANING_TITLE_GAP;
  renderer.drawLine(boxX + MEANING_PADDING, y, boxX + boxWidth - MEANING_PADDING, y, true);
  y += MEANING_TITLE_GAP;
  for (const auto& line : lines) {
    renderer.drawText(NOTOSERIF_14_FONT_ID, boxX + MEANING_PADDING, y, line.c_str());
    y += lineHeight;
  }
}

int TarotActivity::gridItemsPerPage(const ThemeMetrics& metrics) const {
  const int top = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int rows = std::max(1, (contentBottom(renderer, metrics) - top + GRID_GAP_Y) / (THUMB_HEIGHT + GRID_GAP_Y));
  return rows * GRID_COLUMNS;
}

void TarotActivity::renderGrid(const ThemeMetrics& metrics) const {
  const int pageWidth = renderer.getScreenWidth();
  const int perPage = gridItemsPerPage(metrics);
  const int totalPages = (static_cast<int>(history.size()) + perPage - 1) / perPage;
  char pageLabel[32];
  snprintf(pageLabel, sizeof(pageLabel), tr(STR_PAGE_OF_FORMAT), static_cast<unsigned>(gridPageIndex + 1),
           static_cast<unsigned>(totalPages));
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_TAROT_HISTORY),
                 totalPages > 1 ? pageLabel : nullptr);

  const int top = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int gridWidth = GRID_COLUMNS * THUMB_WIDTH + (GRID_COLUMNS - 1) * GRID_GAP_X;
  const int gridX = (pageWidth - gridWidth) / 2;
  const int start = gridPageIndex * perPage;
  for (int i = 0; i < perPage && static_cast<size_t>(start + i) < history.size(); i++) {
    const Rect slot{gridX + (i % GRID_COLUMNS) * (THUMB_WIDTH + GRID_GAP_X),
                    top + (i / GRID_COLUMNS) * (THUMB_HEIGHT + GRID_GAP_Y), THUMB_WIDTH, THUMB_HEIGHT};
    const Rect drawn = drawBmpCentered(renderer, TarotAssets::getCardThumbPath(history[start + i]), slot);
    if (drawn.width > 0) renderer.drawRect(drawn.x, drawn.y, drawn.width, drawn.height, true);
  }

  const bool hasPrev = gridPageIndex > 0;
  const bool hasNext = gridPageIndex < totalPages - 1;
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", hasPrev ? tr(STR_PREV) : "", hasNext ? tr(STR_NEXT) : "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
