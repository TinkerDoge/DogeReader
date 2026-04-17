#include "TarotDeck.h"
#include <algorithm>
#include <esp_random.h>
#include <esp_system.h>

TarotDeck::TarotDeck() : currentIndex(-1) {
    sequence.reserve(DECK_SIZE);
    for (int i = 0; i < DECK_SIZE; ++i) {
        sequence.push_back(i);
    }
    shuffle();
}

void TarotDeck::shuffle() {
    // Fisher-Yates shuffle
    for (int i = DECK_SIZE - 1; i > 0; --i) {
        // Use esp_random() for high-quality randomness
        uint32_t j = esp_random() % (i + 1);
        std::swap(sequence[i], sequence[j]);
    }
    currentIndex = -1; // Reset to "no card drawn yet"
}

int8_t TarotDeck::drawNext() {
    if (hasNext()) {
        currentIndex++;
        return sequence[currentIndex];
    }
    return -1;
}

int8_t TarotDeck::getCurrent() const {
    if (currentIndex >= 0 && currentIndex < DECK_SIZE) {
        return sequence[currentIndex];
    }
    return -1;
}

bool TarotDeck::hasNext() const {
    return currentIndex < DECK_SIZE - 1;
}

void TarotDeck::reset() {
    currentIndex = -1;
}

std::vector<int8_t> TarotDeck::getHistory(int count) const {
    std::vector<int8_t> history;
    if (currentIndex < 0) return history;

    for (int i = 0; i < count && (currentIndex - i) >= 0; ++i) {
        history.push_back(sequence[currentIndex - i]);
    }
    return history;
}

int8_t TarotDeck::getFromHistory(int offset) const {
    int targetIndex = currentIndex - offset;
    if (targetIndex >= 0 && targetIndex < DECK_SIZE) {
        return sequence[targetIndex];
    }
    return -1;
}
