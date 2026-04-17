#pragma once

#include "../Activity.h"
#include "TarotDeck.h"
#include "TarotAssets.h"
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
    bool showMeaning;
    bool firstRenderDone;
    
    // UI state
    int8_t currentCardId;

    void drawNextCard();
    void toggleMeaning();
    void showHistory();
};
