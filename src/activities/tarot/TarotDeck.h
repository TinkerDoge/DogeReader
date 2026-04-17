#pragma once

#include <vector>
#include <cstdint>

class TarotDeck {
public:
    static const int DECK_SIZE = 78;

    TarotDeck();

    // Shuffles the deck and resets the current index
    void shuffle();

    // Returns the ID of the next card (0-77). Returns -1 if no more cards.
    int8_t drawNext();

    // Returns the ID of the current card (the one most recently drawn)
    int8_t getCurrent() const;

    // Returns true if there are cards left to draw in the current shuffled sequence
    bool hasNext() const;

    // Resets the current index to the beginning of the shuffled sequence
    void reset();

    // Returns a list of recently drawn card IDs (up to 'count' items)
    // Most recent is at the beginning of the vector
    std::vector<int8_t> getHistory(int count) const;

    // Returns the ID of the card at a specific position in the history relative to current
    // offset 0 is current, offset 1 is previous, etc.
    int8_t getFromHistory(int offset) const;

    int getCurrentIndex() const { return currentIndex; }

private:
    std::vector<int8_t> sequence;
    int currentIndex; // Index of the card currently being shown (drawn)
};
