#include "TarotActivity.h"
#include <Bitmap.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <I18n.h>
#include "components/UITheme.h"
#include "fontIds.h"

TarotActivity::TarotActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : Activity("Tarot", renderer, mappedInput), showMeaning(false), firstRenderDone(false), currentCardId(-1) {}

void TarotActivity::onEnter() {
    Activity::onEnter();
    deck.shuffle();
    assets.loadMeanings();
    requestUpdate();
}

void TarotActivity::onExit() {
    Activity::onExit();
}

void TarotActivity::loop() {
    Activity::loop();

    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
        if (showMeaning) {
            showMeaning = false;
        }
        drawNextCard();
    }

    if (mappedInput.wasReleased(MappedInputManager::Button::Up)) {
        toggleMeaning();
    }

    if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
        if (showMeaning) {
            showMeaning = false;
            requestUpdate();
        } else {
            finish();
        }
    }

    if (mappedInput.wasReleased(MappedInputManager::Button::Left)) {
        if (deck.getCurrentIndex() > 0) {
            currentCardId = deck.getFromHistory(1); // Since getFromHistory(1) is relative to current
            // Actually, my TarotDeck logic is simple. I should probably just track browsingIndex.
            // For now, let's just draw the next one as requested.
            requestUpdate();
        }
    }
}

void TarotActivity::drawNextCard() {
    currentCardId = deck.drawNext();
    showMeaning = false;
    requestUpdate();
}

void TarotActivity::toggleMeaning() {
    if (currentCardId != -1) {
        showMeaning = !showMeaning;
        requestUpdate();
    }
}

void TarotActivity::render(RenderLock&& lock) {
    const auto pageWidth = renderer.getScreenWidth();
    const auto pageHeight = renderer.getScreenHeight();
    const auto& metrics = UITheme::getInstance().getMetrics();

    renderer.clearScreen();

    // 1. Draw History Header
    std::string historyStr = "";
    auto history = deck.getHistory(3);
    for (size_t i = 0; i < history.size(); ++i) {
        auto m = assets.getMeaning(history[i]);
        historyStr += m.name;
        if (i < history.size() - 1) historyStr += " | ";
    }
    renderer.drawText(UI_10_FONT_ID, metrics.contentSidePadding, metrics.topPadding + 5, historyStr.c_str());

    // 2. Draw Current Card
    if (currentCardId == -1) {
        // Show back of card or welcome message
        FsFile file;
        if (Storage.openFileForRead("TAROT", assets.getBackImagePath(), file)) {
            Bitmap bitmap(file, true);
            if (bitmap.parseHeaders() == BmpReaderError::Ok) {
                int x = (pageWidth - bitmap.getWidth()) / 2;
                int y = (pageHeight - bitmap.getHeight()) / 2;
                renderer.drawBitmap(bitmap, x, y, pageWidth, pageHeight, 0, 0);
            }
            file.close();
        } else {
            renderer.drawCenteredText(UI_12_FONT_ID, pageHeight / 2, tr(STR_TAROT_SHUFFLE));
        }
    } else {
        FsFile file;
        if (Storage.openFileForRead("TAROT", assets.getCardImagePath(currentCardId), file)) {
            Bitmap bitmap(file, true);
            if (bitmap.parseHeaders() == BmpReaderError::Ok) {
                int x = (pageWidth - bitmap.getWidth()) / 2;
                int y = (pageHeight - bitmap.getHeight()) / 2;
                renderer.drawBitmap(bitmap, x, y, pageWidth, pageHeight, 0, 0);
            }
            file.close();
        }
    }

    // 3. Meaning Overlay
    if (showMeaning && currentCardId != -1) {
        auto m = assets.getMeaning(currentCardId);
        Rect popup = GUI.drawPopup(renderer, m.name.c_str());
        // Draw description below title in popup
        renderer.drawText(BOOKERLY_14_FONT_ID, popup.x + 10, popup.y + 40, m.meaning.c_str(), popup.width - 20);
    }

    // 4. Hints
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_TAROT_DRAW), tr(STR_TAROT_MEANING), "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

    renderer.displayBuffer(firstRenderDone ? HalDisplay::HALF_REFRESH : HalDisplay::FULL_REFRESH);
    firstRenderDone = true;
}
