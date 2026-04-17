#pragma once

#include <string>
#include <ArduinoJson.h>

struct CardMeaning {
    std::string name;
    std::string meaning;
};

class TarotAssets {
public:
    TarotAssets();

    // Loads the meanings.json file from the SD card
    bool loadMeanings();

    // Returns the meaning for a specific card ID
    CardMeaning getMeaning(int8_t id) const;

    // Returns the path to the BMP image for a specific card ID
    std::string getCardImagePath(int8_t id) const;
    std::string getCardThumbPath(int8_t id) const;

    // Returns the path to the back of the card image
    std::string getBackImagePath() const;

private:
    JsonDocument meaningsDoc;
    bool loaded;
};
