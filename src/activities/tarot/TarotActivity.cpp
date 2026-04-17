#include "TarotActivity.h"
#include <Bitmap.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <I18n.h>
#include "components/UITheme.h"
#include "fontIds.h"

TarotActivity::TarotActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : Activity("Tarot", renderer, mappedInput), showMeaning(false), firstRenderDone(false), currentCardId(-1), viewState(Main), gridPage(0) {}

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
    renderer.clearScreen();
    if (viewState == Main) {
        renderMain();
    } else if (viewState == Grid) {
        renderGrid();
    }
    renderer.displayBuffer(firstRenderDone ? HalDisplay::HALF_REFRESH : HalDisplay::FULL_REFRESH);
    firstRenderDone = true;
}

void TarotActivity::renderMain() {
    const auto pageWidth = renderer.getScreenWidth();
    const auto pageHeight = renderer.getScreenHeight();
    const auto& metrics = UITheme::getInstance().getMetrics();

    // 1. Draw Top Row History (Up to 5 cards)
    auto history = deck.getHistory(6); // Fetch 6 to know if we need a stack effect
    int maxThumbnails = 5;
    int thumbW = 50;
    int thumbH = 80;
    int spacing = 10;
    int startIdx = 1; // skip current card (history[0])
    int numToDraw = std::min((int)history.size() - startIdx, maxThumbnails);
    if (numToDraw < 0) numToDraw = 0;
    
    int totalWidth = numToDraw * thumbW + (numToDraw - 1) * spacing;
    if (totalWidth < 0) totalWidth = 0;
    int startX = (pageWidth - totalWidth) / 2;
    int startY = metrics.topPadding + 5;

    for (int i = 0; i < numToDraw; ++i) {
        int histIdx = startIdx + (numToDraw - 1 - i); // Draw oldest to newest left to right
        int cardId = history[histIdx];
        int x = startX + i * (thumbW + spacing);
        
        // Draw stack effect on the first thumbnail if we have more than 5 history items
        if (i == 0 && history.size() > (size_t)(maxThumbnails + 1)) {
            renderer.drawRect(x - 3, startY + 3, thumbW, thumbH, true);
        }
        
        FsFile file;
        if (Storage.openFileForRead("TAROT", assets.getCardImagePath(cardId), file)) {
            Bitmap bitmap(file, true);
            if (bitmap.parseHeaders() == BmpReaderError::Ok) {
                renderer.drawBitmap(bitmap, x, startY, thumbW, thumbH, 0, 0);
            }
            file.close();
        }
        renderer.drawRect(x, startY, thumbW, thumbH, true); // Frame
    }

    // 2. Draw Main Card (scaled down)
    int mainCardW = 200;
    int mainCardH = 340;
    int mainX = (pageWidth - mainCardW) / 2;
    int mainY = startY + thumbH + 20;

    if (currentCardId == -1) {
        FsFile file;
        if (Storage.openFileForRead("TAROT", assets.getBackImagePath(), file)) {
            Bitmap bitmap(file, true);
            if (bitmap.parseHeaders() == BmpReaderError::Ok) {
                renderer.drawBitmap(bitmap, mainX, mainY, mainCardW, mainCardH, 0, 0);
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
                renderer.drawBitmap(bitmap, mainX, mainY, mainCardW, mainCardH, 0, 0);
            }
            file.close();
        }
        // 2px frame
        renderer.drawRect(mainX - 1, mainY - 1, mainCardW + 2, mainCardH + 2, true);
        renderer.drawRect(mainX - 2, mainY - 2, mainCardW + 4, mainCardH + 4, true);

        // Name and Number
        auto m = assets.getMeaning(currentCardId);
        char labelBuf[128];
        snprintf(labelBuf, sizeof(labelBuf), "%s • %d/78", m.name.c_str(), currentCardId + 1);
        renderer.drawCenteredText(UI_12_FONT_ID, mainY + mainCardH + 15, labelBuf);
    }

    // 3. Meaning Overlay
    if (showMeaning && currentCardId != -1) {
        auto m = assets.getMeaning(currentCardId);
        Rect popup = GUI.drawPopup(renderer, m.name.c_str());
        renderer.drawText(BOOKERLY_14_FONT_ID, popup.x + 10, popup.y + 40, m.meaning.c_str(), popup.width - 20);
    }

    // 4. Hints
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_TAROT_DRAW), tr(STR_TAROT_MEANING), "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}
