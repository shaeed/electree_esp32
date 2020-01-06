/*

BUTTON MODULE

Copyright (C) 2016-2019 by Xose Pérez <xose dot perez at gmail dot com>

*/

// -----------------------------------------------------------------------------
// BUTTON
// -----------------------------------------------------------------------------

#include "button.h"

typedef struct {
    DebounceEvent * button;
    unsigned long actions;
    unsigned int relayID;
} button_t;

std::vector<button_t> _buttons;

void publishButtonStatus(unsigned char id, uint8_t event) {
    if (id >= _buttons.size()) return;
    char payload[2];
    itoa(event, payload, 10);
    mqttSend(MQTT_TOPIC_BUTTON, id, payload, false, false); // 1st bool = force, 2nd = retain
}

unsigned char buttonAction(unsigned char id, unsigned char event) {
    if (id >= _buttons.size()) return BUTTON_MODE_NONE;
    unsigned long actions = _buttons[id].actions;
    if (event == BUTTON_EVENT_PRESSED) return (actions) & 0x0F;
    if (event == BUTTON_EVENT_CLICK) return (actions >> 4) & 0x0F;
    if (event == BUTTON_EVENT_DBLCLICK) return (actions >> 8) & 0x0F;
    if (event == BUTTON_EVENT_LNGCLICK) return (actions >> 12) & 0x0F;
    if (event == BUTTON_EVENT_LNGLNGCLICK) return (actions >> 16) & 0x0F;
    if (event == BUTTON_EVENT_TRIPLECLICK) return (actions >> 20) & 0x0F;
    return BUTTON_MODE_NONE;
}

unsigned long buttonStore(unsigned long pressed, unsigned long click, unsigned long dblclick, unsigned long lngclick, unsigned long lnglngclick, unsigned long tripleclick) {
    unsigned int value;
    value  = pressed;
    value += click << 4;
    value += dblclick << 8;
    value += lngclick << 12;
    value += lnglngclick << 16;
    value += tripleclick << 20;
    return value;
}

uint8_t mapEvent(uint8_t event, uint8_t count, uint16_t length) {
    if (event == EVENT_PRESSED) return BUTTON_EVENT_PRESSED;
    if (event == EVENT_CHANGED) return BUTTON_EVENT_CLICK;
    if (event == EVENT_RELEASED) {
        if (1 == count) {
            if (length > BUTTON_LNGLNGCLICK_DELAY) return BUTTON_EVENT_LNGLNGCLICK;
            if (length > BUTTON_LNGCLICK_DELAY) return BUTTON_EVENT_LNGCLICK;
            return BUTTON_EVENT_CLICK;
        }
        if (2 == count) return BUTTON_EVENT_DBLCLICK;
        if (3 == count) return BUTTON_EVENT_TRIPLECLICK;
    }
    return BUTTON_EVENT_NONE;
}

void buttonEvent(unsigned int id, unsigned char event) {
    DEBUG_MSG_P(PSTR("[BUTTON] Button #%u event %u\n"), id, event);
    if (event == 0) return;
    
    unsigned char action = buttonAction(id, event);

    //MQTT_SUPPORT
    if (action != BUTTON_MODE_NONE || BUTTON_MQTT_SEND_ALL_EVENTS) {
        publishButtonStatus(id, event);
    }

    if (BUTTON_MODE_TOGGLE == action) {
            relayToggle(_buttons[id].relayID);
    }

    if (BUTTON_MODE_ON == action) {
            relayStatus(_buttons[id].relayID, true);
    }

    if (BUTTON_MODE_OFF == action) {
            relayStatus(_buttons[id].relayID, false);
    }
    
    if (BUTTON_MODE_AP == action) {
        wifiStartAP();
    }
    
    if (BUTTON_MODE_RESET == action) {
        reset();
    }

    if (BUTTON_MODE_FACTORY == action) {
        DEBUG_MSG_P(PSTR("\n\nFACTORY RESET\n\n"));
        //resetSettings();
        reset();
    }

    #if defined(JUSTWIFI_ENABLE_WPS)
        if (BUTTON_MODE_WPS == action) {
            wifiStartWPS();
        }
    #endif // defined(JUSTWIFI_ENABLE_WPS)
    
    #if defined(JUSTWIFI_ENABLE_SMARTCONFIG)
        if (BUTTON_MODE_SMART_CONFIG == action) {
            wifiStartSmartConfig();
        }
    #endif // defined(JUSTWIFI_ENABLE_SMARTCONFIG)
}

void buttonConfigureMqtt(const char * payload){
    //
    DeserializationError err = deserializeJson(jsonDoc, payload);
    if(err){
        DEBUG_MSG_P(PSTR("[BCONF] JSON decode failed (%s)\n"), err.c_str());
        return;
    }

    JsonObject data = jsonDoc[JSON_BCONF];
    byte mode = data[CONF_MODE];
    byte noOfButtons = getSetting(K_NO_OF_BUTTONS, 0).toInt();
    byte btnNo;

    switch (mode) {
    case CONF_MODE_NEW:
        {
            byte gpio = data[CONF_GPIO].as<int>();// | 0xFF;
            byte relayNo = data[CONF_RELAY_NO];   // | RELAY_TYPE_INVERSE;
            setSetting(K_BUTTON_PIN, noOfButtons, gpio);
            setSetting(K_BUTTON_RELAY, noOfButtons, relayNo);
            setSetting(K_NO_OF_BUTTONS, noOfButtons+1);  //Increase the total count of relays
            DEBUG_MSG_P(PSTR("[BCONF] Adding new button. GPIO %d, relay %d. Total now %d"),
                        gpio, relayNo, noOfButtons+1);
        }
        break;
    
    case CONF_MODE_DELETE_ALL:
        setSetting(K_NO_OF_BUTTONS, 0);  //Make 0 realys
        DEBUG_MSG_P(PSTR("[BCONF] Deleting all the buttons."));
        break;

    case CONF_MODE_GET_ALL:
        if(noOfButtons == 0){
            DEBUG_MSG_P(PSTR("[BCONF] No Button added\n"));
        } else {
            for(byte i = 0; i < noOfButtons; i++){
                //
            }
        }
        break;

    case CONF_MODE_GET_SINGLE:
        btnNo = data[CONF_BUTTON_NO];

        if(noOfButtons <= btnNo){
            DEBUG_MSG_P(PSTR("[BCONF] Invalid button number.\n"));
        } else {
            //jsonDoc.clear();
            JsonObject obj = jsonDoc.to<JsonObject>();
            JsonObject rconf = obj.createNestedObject(JSON_RCONF);
            rconf[CONF_MODE] = CONF_MODE_GET_SINGLE;
            rconf[CONF_BUTTON_NO] = btnNo;
            rconf[JSON_CONF_RESPONCE] = true;
            rconf[CONF_GPIO] = getSetting(K_BUTTON_PIN, btnNo, 0xFF);
            rconf[CONF_RELAY_NO] = getSetting(K_BUTTON_RELAY, btnNo, 0xFF);
            //uint16_t size = measureJson(doc);

            mqttSend(MQTT_TOPIC_CONF, jsonDoc.as<String>().c_str());
        }
        break;

    case CONF_MODE_UPDATE_SINGLE:
        btnNo = data[CONF_BUTTON_NO];

        if(noOfButtons <= btnNo){
            DEBUG_MSG_P(PSTR("[BCONF] Invalid button number.\n"));
        } else {
            byte gpio = data[CONF_GPIO].as<int>() | 0xFF;
            byte relNo = data[CONF_RELAY_NO].as<int>() | 0xFF;
            setSetting(K_BUTTON_PIN, btnNo, gpio);
            setSetting(K_BUTTON_RELAY, btnNo, relNo);
            DEBUG_MSG_P(PSTR("[BCONF] Updating button %d. GPIO %d, relay %d."),
                        btnNo, gpio, relNo);
        }
        break;

    default:
        break;
    }

    jsonDoc.clear();
}

void buttonMQTTCallback(unsigned int type, const char * topic, const char * payload) {
    if (type == MQTT_CONNECT_EVENT) {
    }

    if (type == MQTT_MESSAGE_EVENT) {
        String t = mqttMagnitude((char *) topic);

        if(t.startsWith(MQTT_TOPIC_CONF)){
            buttonConfigureMqtt(payload);
        }
    }

    if (type == MQTT_DISCONNECT_EVENT) {
        
    }
}

void buttonSetup() {
    unsigned long btnDelay = getSetting("btnDelay", BUTTON_DBLCLICK_DELAY).toInt();
    char noOfButtons = getSetting(K_NO_OF_BUTTONS, 0).toInt();
    //unsigned int actions = buttonStore(BUTTON1_PRESS, BUTTON1_CLICK, BUTTON1_DBLCLICK, BUTTON1_LNGCLICK, BUTTON1_LNGLNGCLICK, BUTTON1_TRIPLECLICK);
    unsigned int actions = buttonStore(BUTTON_MODE_NONE, BUTTON_MODE_TOGGLE, BUTTON_MODE_TOGGLE, BUTTON_MODE_TOGGLE, BUTTON_MODE_TOGGLE, BUTTON_MODE_TOGGLE);
    
    for(char i = 0; i < noOfButtons; i++) {
       _buttons.push_back( {
           new DebounceEvent(getSetting(K_BUTTON_PIN, i, GPIO_NONE).toInt(),
                            //getSetting(K_BUTTON_MODE, i, BUTTON_MODE_TOGGLE).toInt(),
                            2,
                            BUTTON_DEBOUNCE_DELAY,
                            btnDelay),
           actions, 
           getSetting(K_BUTTON_RELAY, i, GPIO_NONE).toInt() //Relay number
           });
    }

    //_buttons.push_back({new DebounceEvent(13, BUTTON_MODE_TOGGLE, BUTTON_DEBOUNCE_DELAY, btnDelay), actions, 0});
    //_buttons.push_back({new DebounceEvent(12, BUTTON_MODE_TOGGLE, BUTTON_DEBOUNCE_DELAY, btnDelay), actions, 1});
    //_buttons.push_back({new DebounceEvent(14, BUTTON_MODE_TOGGLE, BUTTON_DEBOUNCE_DELAY, btnDelay), actions, 2});
    //_buttons.push_back({new DebounceEvent(27, BUTTON_MODE_TOGGLE, BUTTON_DEBOUNCE_DELAY, btnDelay), actions, 3});

    DEBUG_MSG_P(PSTR("[BUTTON] Number of buttons: %u\n"), _buttons.size());

    // Register loop
    espurnaRegisterLoop(buttonLoop);
    mqttRegister(buttonMQTTCallback);
}

void buttonLoop() {
    for (unsigned int i=0; i < _buttons.size(); i++) {
        if (unsigned char event = _buttons[i].button->loop()) {
            unsigned char count = _buttons[i].button->getEventCount();
            unsigned long length = _buttons[i].button->getEventLength();
            unsigned char mapped = mapEvent(event, count, length);
            buttonEvent(i, mapped);
        }
    }
}
