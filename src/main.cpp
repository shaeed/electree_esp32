/*

ELECTREE MAIN MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>

#include "prototypes.h"
#include "wifi_s.h"
#include "debug.h"
#include "settings.h"
#include "utils.h"
#include "mqtt.h"
#include "uartmqtt.h"
#include "button.h"
#include "relay.h"
#include "led2812b.h"

std::vector<void (*)()> _loop_callbacks;
std::vector<void (*)()> _reload_callbacks;
DynamicJsonDocument jsonDoc(JSON_CAPACITY);

void espurnaRegisterLoop(void (*callback)()) {
    _loop_callbacks.push_back(callback);
}

void espurnaRegisterReload(void (*callback)()) {
    _reload_callbacks.push_back(callback);
}

void espurnaReload() {
    for (unsigned char i = 0; i < _reload_callbacks.size(); i++) {
        (_reload_callbacks[i])();
    }
}

void setup() {
  debugSetup();
  settingsSetup();

  if (getSetting("hostname").length() == 0) {
    setDefaultHostname();
  }

  info();
  relaySetup();
  wifiSetup();
  mqttSetup();
  uartmqttSetup();
  buttonSetup();
  led2812bSetup();
}

void loop() {
  // Call registered loop callbacks
  for (unsigned char i = 0; i < _loop_callbacks.size(); i++) {
    (_loop_callbacks[i])();
  }

  //Main loop delay to save power, will not affect the performance
  delay(LOOP_DELAY_TIME);
}