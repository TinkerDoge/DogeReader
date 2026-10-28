#include "TarotDeck.h"

#include <esp_random.h>

#include <utility>

TarotDeck::TarotDeck() {
  for (int i = 0; i < DECK_SIZE; ++i) {
    sequence[i] = static_cast<int8_t>(i);
  }
  shuffle();
}

void TarotDeck::shuffle() {
  // Fisher-Yates with the hardware RNG.
  for (int i = DECK_SIZE - 1; i > 0; --i) {
    const uint32_t j = esp_random() % static_cast<uint32_t>(i + 1);
    std::swap(sequence[i], sequence[j]);
  }
  drawnCount = 0;
}

int8_t TarotDeck::drawNext() {
  if (!hasNext()) return -1;
  return sequence[drawnCount++];
}
