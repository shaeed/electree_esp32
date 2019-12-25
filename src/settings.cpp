/*

SETTINGS MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include "settings.h"


// -----------------------------------------------------------------------------
// Key-value API
// -----------------------------------------------------------------------------

String getSetting(const String& key) {
    return getSetting(key, "");
}

bool delSetting(const String& key) {
    return Embedis::del(key);
}

bool delSetting(const String& key, unsigned int index) {
    return delSetting(key + String(index));
}

bool hasSetting(const String& key) {
    return getSetting(key).length() != 0;
}

bool hasSetting(const String& key, unsigned int index) {
    return getSetting(key, index, "").length() != 0;
}

// -----------------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------------

void settingsSetup() {
    EEPROM.begin(SPI_FLASH_SEC_SIZE);
    Embedis::dictionary( F("EEPROM"),
        SPI_FLASH_SEC_SIZE,
        [](size_t pos) -> char { return EEPROM.read(pos); },
        [](size_t pos, char value) { EEPROM.write(pos, value); },
        []() { EEPROM.commit(); }
    );
}