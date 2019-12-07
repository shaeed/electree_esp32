/*

RELAY MODULE

Copyright (C) 2019 by Shaeed Khan

*/
#include "relay.h"

typedef struct {
    unsigned char pin;          // GPIO pin for the relay
    unsigned char type;         // RELAY_TYPE_NORMAL, RELAY_TYPE_INVERSE, RELAY_TYPE_LATCHED or RELAY_TYPE_LATCHED_INVERSE

    // Status variables
    bool current_status;        // Holds the current (physical) status of the relay
    bool target_status;         // Holds the target status
    unsigned long fw_start;     // Flood window start time
    unsigned char fw_count;     // Number of changes within the current flood window
    unsigned long change_time;  // Scheduled time to change
    bool report;                // Whether to report to own topic
    bool group_report;          // Whether to report to group topic
} relay_t;

std::vector<relay_t> _relays;
bool _relayRecursive = false;
Ticker _relaySaveTicker;

// -----------------------------------------------------------------------------
// RELAY PROVIDERS
// -----------------------------------------------------------------------------

void _relayProviderStatus(unsigned char id, bool status) {
    // Check relay ID
    if (id >= _relays.size()) return;

    // Store new current status
    _relays[id].current_status = status;

    if (_relays[id].type == RELAY_TYPE_NORMAL) {
        digitalWrite(_relays[id].pin, status);
    } else if (_relays[id].type == RELAY_TYPE_INVERSE) {
        digitalWrite(_relays[id].pin, !status);
    } else { 
        DEBUG_MSG_P(PSTR("[RELAY] Invalid type for #%d, type is %d\n"), id, _relays[id].type);
    }
}

/**
 * Walks the relay vector processing only those relays
 * that have to change to the requested mode
 * @bool mode Requested mode
 */
void _relayProcess(bool mode) {
    unsigned long current_time = millis();

    for (unsigned char id = 0; id < _relays.size(); id++) {
        bool target = _relays[id].target_status;

        // Only process the relays we have to change
        if (target == _relays[id].current_status) continue;

        // Only process the relays we have to change to the requested mode
        if (target != mode) continue;

        // Only process if the change_time has arrived
        if (current_time < _relays[id].change_time) continue;

        DEBUG_MSG_P(PSTR("[RELAY] #%d set to %s\n"), id, target ? "ON" : "OFF");

        // Call the provider to perform the action
        _relayProviderStatus(id, target);

        // Send MQTT
        publishRelayStatus(id);

        if (!_relayRecursive) {
            unsigned char boot_mode = getSetting(K_RELAY_BOOT_MODE, id, RELAY_BOOT_MODE).toInt();
            bool do_commit = ((RELAY_BOOT_SAME == boot_mode) || (RELAY_BOOT_TOGGLE == boot_mode));
            //relaySave(do_commit);
            _relaySaveTicker.once_ms(RELAY_SAVE_DELAY, relaySave, do_commit);
        }

        _relays[id].report = false;
        _relays[id].group_report = false;
    }
}

bool relayStatus(unsigned char id, bool status, bool report, bool group_report) {
    if (id >= _relays.size()) return false;
    
    bool changed = false;

    if (_relays[id].current_status == status) {

    } else {
        unsigned long current_time = millis();
        unsigned long fw_end = _relays[id].fw_start + 1000 * RELAY_FLOOD_WINDOW;
        //unsigned long delay = status ? _relays[id].delay_on : _relays[id].delay_off;

        _relays[id].fw_count++;
        _relays[id].change_time = current_time;

        // If current_time is off-limits the floodWindow...
        if (current_time < _relays[id].fw_start || fw_end <= current_time) {
            // We reset the floodWindow
            _relays[id].fw_start = current_time;
            _relays[id].fw_count = 1;

        // If current_time is in the floodWindow and there have been too many requests...
        } else if (_relays[id].fw_count >= RELAY_FLOOD_CHANGES) {
            // We schedule the changes to the end of the floodWindow
            // unless it's already delayed beyond that point
            if (fw_end > current_time) {
                _relays[id].change_time = fw_end;
            }
        }

        _relays[id].target_status = status;
        if (report) _relays[id].report = true;
        if (group_report) _relays[id].group_report = true;

        DEBUG_MSG_P(PSTR("[RELAY] #%d scheduled %s in %u ms\n"),
                id, status ? "ON" : "OFF",
                (_relays[id].change_time - current_time));

        changed = true;
    }

    return changed;
}

bool relayStatus(unsigned char id, bool status) {
    return relayStatus(id, status, true, true);
}

bool relayStatus(unsigned char id) {
    // Check relay ID
    if (id >= _relays.size()) return false;

    // Get status from storage
    return _relays[id].current_status;
}

void relaySave(bool do_commit) {
    // Relay status is stored in a single byte
    // This means that, it will be stored in group of 8
    unsigned char sizeOfCurrentBatch;
    unsigned char bit = 1;
    unsigned char mask = 0;
    unsigned char count = _relays.size();
    unsigned char currentRelay;
    bool save = false;
    byte bootMode = RELAY_BOOT_OFF;

    for(unsigned char j = 0; j <= _relays.size() / 8; j++){
        sizeOfCurrentBatch = _relays.size() > 8*(j+1) ? 8 : _relays.size() - 8*j;
        bit = 1;
        mask = 0;
        save = false;

        for (unsigned char i = 0; i < sizeOfCurrentBatch; i++) {
            currentRelay = i + 8*j;
            bootMode = getSetting(K_RELAY_BOOT_MODE, RELAY_BOOT_MODE).toInt();
            if (relayStatus(currentRelay) &&
                (bootMode == RELAY_BOOT_SAME || bootMode == RELAY_BOOT_TOGGLE)) {
                mask += bit;
            }
            bit += bit;
        }
        
        if(do_commit && getSetting(K_RELAY_STATUS_ALL, j, 0).toInt() != mask){
            setSetting(K_RELAY_STATUS_ALL, j, mask);
            DEBUG_MSG_P(PSTR("[RELAY] Setting relay mask: %d\n"), mask);
        }
    }//End outer for loop
}

void relaySave() {
    relaySave(true);
}

void relayToggle(unsigned char id, bool report, bool group_report) {
    if (id >= _relays.size()) return;
    relayStatus(id, !relayStatus(id), report, group_report);
}

void relayToggle(unsigned char id) {
    relayToggle(id, true, true);
}

unsigned char relayCount() {
    return _relays.size();
}

unsigned char relayParsePayload(const char * payload) {
    //
    if (payload[0] == '0') return 0;
    if (payload[0] == '1') return 1;
    if (payload[0] == '2') return 2;
    if (payload[0] == '3') return 3;

    return 0xFF;
}

void _relayBoot() {

    _relayRecursive = true;
    bool trigger_save = false;
    unsigned char bit = 1;
    unsigned char mask;
    
    // Walk the relays
    Serial.print("Relay boot" );
    Serial.print(_relays.size());

    bool status;
    for(unsigned char j = 0; j <= _relays.size() / 8; j++){
        Serial.print(" inside ");
        Serial.print(j);

        unsigned char sizeOfCurrentBatch = _relays.size() > 8*(j+1) ? 8 : _relays.size() - 8*j;
        bit = 1;
        mask = getSetting(K_RELAY_STATUS_ALL, j, 0x00).toInt();
        DEBUG_MSG_P(PSTR("[RELAY] Retrieving mask: %d\n"), mask);
        trigger_save = false;

        Serial.print(" batch size ");
        Serial.print(sizeOfCurrentBatch);

        for (unsigned char i = 0; i < sizeOfCurrentBatch; i++) {
            unsigned char currentRelay = i + 8*j;
            unsigned char boot_mode = getSetting(K_RELAY_BOOT_MODE, currentRelay, RELAY_BOOT_MODE).toInt();
            DEBUG_MSG_P(PSTR("[RELAY] Relay #%d boot mode %d\n"), currentRelay, boot_mode);

            status = false;
            switch (boot_mode) {
                case RELAY_BOOT_SAME:
                    status = ((mask & bit) == bit);
                    break;
                case RELAY_BOOT_TOGGLE:
                    status = ((mask & bit) != bit);
                    mask ^= bit;
                    trigger_save = true;
                    break;
                case RELAY_BOOT_ON:
                    status = true;
                    break;

                case RELAY_BOOT_OFF:
                default:
                    break;
            }

            _relays[currentRelay].current_status = !status;
            _relays[currentRelay].target_status = status;
            //_relays[currentRelay].change_time = millis();

            bit <<= 1;
        }

        // Save if there is any relay in the RELAY_BOOT_TOGGLE mode
        if (trigger_save) {
            //EEPROMr.write(EEPROM_RELAY_STATUS, mask);
            //eepromCommit();
            setSetting(K_RELAY_STATUS_ALL, j, mask);
        }
    }

    _relayRecursive = false;
}

void _relayConfigure() {
    for (char i = 0; i < _relays.size(); i++) {
        if (GPIO_NONE == _relays[i].pin) continue;

        pinMode(_relays[i].pin, OUTPUT);
        if (_relays[i].type == RELAY_TYPE_INVERSE) {
            //set to high to block short opening of relay
            digitalWrite(_relays[i].pin, HIGH);
        }
    }
}

//------------------------------------------------------------------------------
// MQTT
//------------------------------------------------------------------------------

void publishRelayStatus(unsigned char id) {
    if (id >= _relays.size()) return;

    // Send state topic
    if (_relays[id].report) {
        _relays[id].report = false;
        mqttSend(MQTT_TOPIC_RELAY, id, _relays[id].current_status ? RELAY_MQTT_ON : RELAY_MQTT_OFF);
    }
}

void publishRelayStatus() {
    for (unsigned int id=0; id < _relays.size(); id++) {
        mqttSend(MQTT_TOPIC_RELAY, id, _relays[id].current_status ? RELAY_MQTT_ON : RELAY_MQTT_OFF);
    }
}

void relayStatusWrap(unsigned char id, unsigned char value) {
    switch (value) {
        case 0: //Off
            relayStatus(id, false, true, false);
            break;
        case 1: //On
            relayStatus(id, true, true, false);
            break;
        case 2: //Toggle
            relayToggle(id, true, true);
            break;
        default: //Publish status
            _relays[id].report = true;
            publishRelayStatus(id);
            break;
    }
}

void relayConfigureMqtt(const char * payload){
    //
    DeserializationError err = deserializeJson(jsonDoc, payload);
    if(err){
        DEBUG_MSG_P(PSTR("[RCONF] JSON decode failed (%s)\n"), err.c_str());
        return;
    }

    JsonObject data = jsonDoc[JSON_RCONF];
    byte mode = data[CONF_MODE];
    byte noOfRelays = getSetting(K_NO_OF_RELAYS, 0).toInt();
    byte relayNo;

    switch (mode) {
    case CONF_MODE_NEW:
        {
            byte gpio = data[CONF_GPIO].as<int>();// | 0xFF;
            byte type = data[CONF_TYPE].as<int>();// | RELAY_TYPE_INVERSE;
            byte bootMode = data[CONF_RELAY_BOOT_MODE].as<int>();
            setSetting(K_RELAY_PIN, noOfRelays, gpio);
            setSetting(K_RELAY_TYPE, noOfRelays, type);
            setSetting(K_RELAY_BOOT_MODE, noOfRelays, bootMode);
            setSetting(K_NO_OF_RELAYS, noOfRelays+1);  //Increase the total count of relays
            DEBUG_MSG_P(PSTR("[RCONF] Adding new relay. GPIO %d, type %d. Total now %d"),
                        gpio, type, noOfRelays+1);
        }
        break;
    
    case CONF_MODE_DELETE_ALL:
        setSetting(K_NO_OF_RELAYS, 0);  //Make 0 realys
        DEBUG_MSG_P(PSTR("[RCONF] Deleting all the relays."));
        break;

    case CONF_MODE_GET_ALL:
        if(noOfRelays == 0){
            DEBUG_MSG_P(PSTR("[RCONF] No realay added\n"));
        } else {
            for(byte i = 0; i < noOfRelays; i++){
                //
            }
        }
        break;

    case CONF_MODE_GET_SINGLE:
        relayNo = data[CONF_RELAY_NO];

        if(noOfRelays <= relayNo){
            DEBUG_MSG_P(PSTR("[RCONF] Invalid relay number.\n"));
        } else {
            //jsonDoc.clear();
            JsonObject obj = jsonDoc.to<JsonObject>();
            JsonObject rconf = obj.createNestedObject(JSON_RCONF);
            rconf[CONF_MODE] = CONF_MODE_GET_SINGLE;
            rconf[CONF_RELAY_NO] = relayNo;
            rconf[JSON_CONF_RESPONCE] = true;
            rconf[CONF_GPIO] = getSetting(K_RELAY_PIN, relayNo, 0xFF);
            rconf[CONF_TYPE] = getSetting(K_RELAY_TYPE, relayNo, RELAY_TYPE_NORMAL);
            rconf[CONF_RELAY_BOOT_MODE] = getSetting(K_RELAY_BOOT_MODE, relayNo, RELAY_BOOT_MODE);
            //uint16_t size = measureJson(doc);

            mqttSend(MQTT_TOPIC_CONF, jsonDoc.as<String>().c_str());
        }
        break;

    case CONF_MODE_UPDATE_SINGLE:
        relayNo = data[CONF_RELAY_NO];

        if(noOfRelays <= relayNo){
            DEBUG_MSG_P(PSTR("[RCONF] Invalid relay number.\n"));
        } else {
            byte gpio = data[CONF_GPIO].as<int>();
            byte type = data[CONF_TYPE].as<int>();
            byte bootMode = data[CONF_RELAY_BOOT_MODE].as<int>();
            setSetting(K_RELAY_PIN, relayNo, gpio);
            setSetting(K_RELAY_TYPE, relayNo, type);
            setSetting(K_RELAY_BOOT_MODE, relayNo, bootMode);
            DEBUG_MSG_P(PSTR("[RCONF] Updating relay %d. GPIO %d, type %d."),
                        relayNo, gpio, type);
        }
        break;

    default:
        break;
    }

    jsonDoc.clear();
}

void relayMQTTCallback(unsigned int type, const char * topic, const char * payload) {
    if (type == MQTT_CONNECT_EVENT) {
        // Send status on connect
        publishRelayStatus();
        // Subscribe to own /set topic
        char relay_topic[strlen(MQTT_TOPIC_RELAY) + 3];
        snprintf_P(relay_topic, sizeof(relay_topic), PSTR("%s/+"), MQTT_TOPIC_RELAY);
        mqttSubscribe(relay_topic);
    }

    if (type == MQTT_MESSAGE_EVENT) {
        String t = mqttMagnitude((char *) topic);

        if (t.startsWith(MQTT_TOPIC_RELAY)) {
            // magnitude is relay/#
            // Get relay ID
            unsigned int id = t.substring(strlen(MQTT_TOPIC_RELAY)+1).toInt();
            if (id >= relayCount()) {
                DEBUG_MSG_P(PSTR("[RELAY] Wrong relayID (%d)\n"), id);
                return;
            }

            // Get value
            unsigned char value = relayParsePayload(payload);
            if (value == 0xFF) return;
            relayStatusWrap(id, value);

        } else if(t.startsWith(MQTT_TOPIC_CONF)){
            //Configure relay from mqtt
            relayConfigureMqtt(payload);
        }
        // Itead Sonoff IFAN02
        /*#if defined (ITEAD_SONOFF_IFAN02)
            if (t.startsWith(MQTT_TOPIC_SPEED)) {
                setSpeed(atoi(payload));
            }
        #endif*/
    }

    if (type == MQTT_DISCONNECT_EVENT) {
        
    }
}

void relaySetupMQTT() {
    mqttRegister(relayMQTTCallback);
}

void _relayLoop() {
    _relayProcess(false);
    _relayProcess(true);
}

void relaySetup() {
    //Number of relays
    char noOfRelays = getSetting(K_NO_OF_RELAYS, 0).toInt();
    for(char i = 0; i < noOfRelays; i++) {
        _relays.push_back((relay_t) { getSetting(K_RELAY_PIN, i, GPIO_NONE).toInt(),
                                    getSetting(K_RELAY_TYPE, i, RELAY_TYPE_INVERSE).toInt(),
                                    //GPIO_NONE,
                                    //RELAY_DELAY_ON,
                                    //RELAY_DELAY_OFF 
                                    });
    }

    _relayConfigure();
    _relayBoot();
    _relayLoop();

    relaySetupMQTT();

    // Main callbacks
    espurnaRegisterLoop(_relayLoop);
    espurnaRegisterReload(_relayConfigure);

    DEBUG_MSG_P(PSTR("[RELAY] Number of relays: %d\n"), _relays.size());
}
