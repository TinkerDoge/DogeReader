#include "TarotAssets.h"
#include <HalStorage.h>
#include <Logging.h>

TarotAssets::TarotAssets() : loaded(false) {}

bool TarotAssets::loadMeanings() {
    if (loaded) return true;

    if (!Storage.exists("/tarot/meanings.json")) {
        LOG_ERR("TAROT", "meanings.json not found in /tarot/");
        return false;
    }

    String json = Storage.readFile("/tarot/meanings.json");
    if (json.length() == 0) {
        LOG_ERR("TAROT", "Failed to read meanings.json");
        return false;
    }

    auto error = deserializeJson(meaningsDoc, json);
    if (error) {
        LOG_ERR("TAROT", "JSON parse error: %s", error.c_str());
        return false;
    }

    loaded = true;
    return true;
}

CardMeaning TarotAssets::getMeaning(int8_t id) const {
    CardMeaning result = {"Unknown", "No meaning found."};
    
    if (!loaded) return result;

    JsonArrayConst cards = meaningsDoc["cards"].as<JsonArrayConst>();
    for (JsonObjectConst card : cards) {
        if (card["id"] == id) {
            result.name = card["name"] | "Unknown";
            result.meaning = card["desc"] | "No meaning found.";
            break;
        }
    }
    
    return result;
}

std::string TarotAssets::getCardImagePath(int8_t id) const {
    char path[32];
    snprintf(path, sizeof(path), "/tarot/cards/%d.bmp", id);
    return std::string(path);
}

std::string TarotAssets::getBackImagePath() const {
    return "/tarot/back.bmp";
}
