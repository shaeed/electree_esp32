/*

UTILS MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include "utils.h"

Ticker _defer_reset;
uint8_t _reset_reason = 0;

String getIdentifier() {
    char buffer[20];
    
    #if defined(ARDUINO_ARCH_ESP32)
        snprintf_P(buffer, sizeof(buffer), PSTR("%s-%06X"), APP_NAME, ESP.getEfuseMac() & 0xFFFFFFFF);
    #elif defined(ARDUINO_ARCH_ESP8266)
        snprintf_P(buffer, sizeof(buffer), PSTR("%s-%06X"), APP_NAME, ESP.getChipId());
    #endif
    return String(buffer);
}

void setDefaultHostname() {
    if (strlen(HOSTNAME) > 0) {
        setSetting("hostname", HOSTNAME);
    } else {
        setSetting("hostname", getIdentifier());
    }
}

String getBoardName() {
    return getSetting("boardName", DEVICE_NAME);
}

String getAdminPass() {
    return getSetting("adminPass", ADMIN_PASS);
}



void info() {
    DEBUG_MSG_P(PSTR("\n\n---8<-------\n\n"));

    // -------------------------------------------------------------------------
    #if defined(ARDUINO_ARCH_ESP32)
        DEBUG_MSG_P(PSTR("[MAIN] CPU chip ID: 0x%06X\n"), ESP.getEfuseMac() & 0xFFFFFFFF);
    #elif defined(ARDUINO_ARCH_ESP8266)
        DEBUG_MSG_P(PSTR("[MAIN] CPU chip ID: 0x%06X\n"), ESP.getChipId());
    #endif
    
    DEBUG_MSG_P(PSTR("[MAIN] CPU frequency: %u MHz\n"), ESP.getCpuFreqMHz());
    DEBUG_MSG_P(PSTR("[MAIN] SDK version: %s\n"), ESP.getSdkVersion());
    DEBUG_MSG_P(PSTR("\n"));
    DEBUG_MSG_P(PSTR("\n\n---8<-------\n\n"));
}


// -----------------------------------------------------------------------------
// Reset
// -----------------------------------------------------------------------------

void reset() {
    ESP.restart();
}

// -----------------------------------------------------------------------------

char * ltrim(char * s) {
    char *p = s;
    while ((unsigned char) *p == ' ') ++p;
    return p;
}

double roundTo(double num, unsigned char positions) {
    double multiplier = 1;
    while (positions-- > 0) multiplier *= 10;
    return round(num * multiplier) / multiplier;
}

void nice_delay(unsigned long ms) {
    unsigned long start = millis();
    while (millis() - start < ms) delay(1);
}

// This method is called by the SDK to know where to connect the ADC
/*int __get_adc_mode() {
    return (int) (ADC_MODE_VALUE);
}*/

bool isNumber(const char * s) {
    unsigned char len = strlen(s);
    if (0 == len) return false;
    bool decimal = false;
    bool digit = false;
    for (unsigned char i=0; i<len; i++) {
        if (('-' == s[i]) || ('+' == s[i])) {
            if (i>0) return false;
        } else if (s[i] == '.') {
            if (!digit) return false;
            if (decimal) return false;
            decimal = true;
        } else if (!isdigit(s[i])) {
            return false;
        } else {
            digit = true;
        }
    }
    return digit;
}
