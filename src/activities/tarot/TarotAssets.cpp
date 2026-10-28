#include "TarotAssets.h"

#include <HalStorage.h>
#include <Logging.h>

#include <cstdio>

#include "util/HalFileJsonReader.h"

namespace {
constexpr char MEANINGS_PATH[] = "/tarot/meanings.json";
}

bool TarotAssets::loadMeanings() {
  if (loaded) return true;

  HalFile file;
  if (!Storage.exists(MEANINGS_PATH) || !Storage.openFileForRead("TAROT", MEANINGS_PATH, file)) {
    LOG_ERR("TAROT", "%s not found", MEANINGS_PATH);
    return false;
  }

  HalFileJsonReader reader{file};
  const auto error = deserializeJson(meaningsDoc, reader);
  if (error) {
    LOG_ERR("TAROT", "JSON parse error: %s", error.c_str());
    meaningsDoc.clear();
    return false;
  }

  loaded = true;
  return true;
}

CardMeaning TarotAssets::getMeaning(const int8_t id) const {
  CardMeaning result = {"", ""};
  if (!loaded) return result;

  for (JsonObjectConst card : meaningsDoc.as<JsonArrayConst>()) {
    if (card["id"] == id) {
      result.name = card["name"] | "";
      result.meaning = card["meaning"] | "";
      break;
    }
  }
  return result;
}

std::string TarotAssets::getCardImagePath(const int8_t id) {
  char path[32];
  snprintf(path, sizeof(path), "/tarot/cards/%d.bmp", id);
  return std::string(path);
}

std::string TarotAssets::getCardThumbPath(const int8_t id) {
  char path[32];
  snprintf(path, sizeof(path), "/tarot/thumbs/%d.bmp", id);
  return std::string(path);
}
