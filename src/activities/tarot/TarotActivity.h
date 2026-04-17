#pragma once

#include "../Activity.h"
#include "TarotDeck.h"
#include "TarotAssets.h"
#include "components/themes/BaseTheme.h"
#include "util/ButtonNavigator.h"

class TarotActivity : public Activity {
public:
    TarotActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);

    void onEnter() override;
    void onExit() override;
    void loop() override;
    void render(RenderLock&& lock) override;

private:
    TarotDeck deck;
    TarotAssets assets;
    enum class ViewState { DrawPrompt, Main, HistoryGrid };
    ViewState viewState = ViewState::DrawPrompt;

    bool showMeaning;
    bool firstRenderDone;
    bool isDrawing = false;
    
    // UI state
    int8_t currentCardId;
    std::vector<int8_t> history;
    int gridPageIndex = 0;

    void drawNextCard();
    void toggleMeaning();
    void renderMain(const ThemeMetrics& metrics, int pageWidth, int pageHeight);
    void renderGrid(const ThemeMetrics& metrics, int pageWidth, int pageHeight);
    void renderPrompt(const ThemeMetrics& metrics, int pageWidth, int pageHeight);
};
