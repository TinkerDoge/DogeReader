#pragma once

#include <ArduinoJson.h>

#include <cstdint>
#include <string>

// Points into TarotAssets' JSON document; valid while the TarotAssets lives.
struct CardMeaning {
  const char* name;
  const char* meaning;
};

// Card meanings and image paths from /tarot on the SD card.
class TarotAssets {
 public:
  // Loads /tarot/meanings.json. Returns false if the deck is not installed.
  bool loadMeanings();
  bool isLoaded() const { return loaded; }

  CardMeaning getMeaning(int8_t id) const;

  static std::string getCardImagePath(int8_t id);
  static std::string getCardThumbPath(int8_t id);
  static const char* getBackImagePath() { return "/tarot/back.bmp"; }

 private:
  JsonDocument meaningsDoc;
  bool loaded = false;
};
