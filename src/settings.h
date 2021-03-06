/*

SETTINGS HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <vector>
#include <ArduinoJson.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <esp_spi_flash.h>
#endif

#include <EEPROM.h>

#include "Embedis.h"

#include "utils.h"

#define K_HOSTNAME        "z"

#define K_NO_OF_RELAYS    "a"
#define K_RELAY_PIN       "b"
#define K_RELAY_TYPE      "c"
#define K_RELAY_STATUS_ALL "d"
#define K_RELAY_BOOT_MODE  "e"

#define K_NO_OF_BUTTONS    "f"
#define K_BUTTON_PIN       "g"
#define K_BUTTON_MODE      "h"
#define K_BUTTON_RELAY     "i"   //Relay number(0 based), not relay GPIO

#define K_NO_OF_LEDSTRIPS  "j"
#define K_LEDSTRIP_PIN     "k"
#define K_LEDSTRIP_LEDCOUNT "l"
#define K_LEDSTRIP_BRIGHTNESS_ALL "m"
#define K_LEDSTRIP_SPD     "n"
#define K_LEDSTRIP_CLR     "o"
#define K_LEDSTRIP_THM     "p"

#define K_MQTT_SERVER      "q"
#define K_MQTT_PORT        "qa"
#define K_MQTT_USER        "qb"
#define K_MQTT_PASS        "qc"
#define K_MQTT_CLT_ID      "qd"
#define K_MQTT_USE_SSL     "qe"
#define K_MQTT_FP          "qf" //MQTT_SSL_FINGERPRINT
#define K_MQTT_TOPIC       "qg" 
#define K_MQTT_SETTER      "qh" 
#define K_MQTT_GETTER      "qi" 
#define K_MQTT_QOS         "qj" 
#define K_MQTT_RETAIN      "qk" 
#define K_MQTT_KEEP        "ql" 
#define K_MQTT_ENABLED     "qm" 

template<typename T> String getSetting(const String& key, T defaultValue);
template<typename T> String getSetting(const String& key, unsigned int index, T defaultValue);
String getSetting(const String& key);
template<typename T> bool setSetting(const String& key, T value);
template<typename T> bool setSetting(const String& key, unsigned int index, T value);
bool delSetting(const String& key);
bool delSetting(const String& key, unsigned int index);
bool hasSetting(const String& key);
bool hasSetting(const String& key, unsigned int index);
void settingsSetup();


template<typename T> String getSetting(const String& key, T defaultValue) {
    String value;
    if (!Embedis::get(key, value)) value = String(defaultValue);
    return value;
}

template<typename T> String getSetting(const String& key, unsigned int index, T defaultValue) {
    return getSetting(key + String(index), defaultValue);
}

template<typename T> bool setSetting(const String& key, T value) {
    return Embedis::set(key, String(value));
}

template<typename T> bool setSetting(const String& key, unsigned int index, T value) {
    return setSetting(key + String(index), value);
}

#endif