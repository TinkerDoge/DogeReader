#pragma once

#include <array>
#include <cstdint>

// A shuffled 78-card deck drawn one card at a time.
class TarotDeck {
 public:
  static constexpr int DECK_SIZE = 78;

  TarotDeck();

  // Shuffles all 78 cards and starts drawing from the top again.
  void shuffle();

  // Returns the next card ID (0-77), or -1 once every card has been drawn.
  int8_t drawNext();

  bool hasNext() const { return drawnCount < DECK_SIZE; }
  int getDrawnCount() const { return drawnCount; }

 private:
  std::array<int8_t, DECK_SIZE> sequence{};
  int drawnCount = 0;
};
