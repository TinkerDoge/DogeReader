#pragma once
#include <cstdint>

// Measures active reading time for one reader session. Call tick() whenever
// the page changes; each gap between ticks counts toward reading time, capped
// at MAX_GAP_MS so a book left open until auto-sleep does not log idle time.
class ReadingSessionTimer {
 public:
  static constexpr uint32_t MAX_GAP_MS = 5UL * 60 * 1000;

  void start(const uint32_t nowMs) {
    lastTickMs = nowMs;
    activeMs = 0;
    running = true;
  }

  void tick(const uint32_t nowMs) {
    if (!running) return;
    const uint32_t gap = nowMs - lastTickMs;  // unsigned math survives millis() wrap
    activeMs += gap < MAX_GAP_MS ? gap : MAX_GAP_MS;
    lastTickMs = nowMs;
  }

  // Stops the timer and returns active minutes, rounded to the nearest minute.
  uint32_t finishMinutes(const uint32_t nowMs) {
    tick(nowMs);
    running = false;
    return (activeMs + 30000) / 60000;
  }

 private:
  uint32_t lastTickMs = 0;
  uint32_t activeMs = 0;
  bool running = false;
};
