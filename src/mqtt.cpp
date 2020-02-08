/*

MQTT MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#include "mqtt.h"

AsyncMqttClient _mqtt;

bool _mqtt_enabled = true;
unsigned long _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;
unsigned long _mqtt_last_connection = 0;
bool _mqtt_connecting = false;
unsigned char _mqtt_qos = MQTT_QOS;
bool _mqtt_retain = MQTT_RETAIN;
unsigned long _mqtt_keepalive = MQTT_KEEPALIVE;
String _mqtt_topic_base;
String _mqtt_setter;
String _mqtt_getter;
bool _mqtt_forward;
char *_mqtt_user = 0;
char *_mqtt_pass = 0;
char *_mqtt_will;
char *_mqtt_clientid;

std::vector<mqtt_callback_f> _mqtt_callbacks;

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------

void _mqttConnect() {
    // Do not connect if already connected or still trying to connect
    if (isMqttConnected() || _mqtt_connecting) return;

    // Check reconnect interval
    if (millis() - _mqtt_last_connection < _mqtt_reconnect_delay) return;

    // Increase the reconnect delay
    _mqtt_reconnect_delay += MQTT_RECONNECT_DELAY_STEP;
    if (_mqtt_reconnect_delay > MQTT_RECONNECT_DELAY_MAX) {
        _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MAX;
    }

    String h = getSetting(K_MQTT_SERVER, MQTT_SERVER);
    char * host = strdup(h.c_str());

    unsigned int port = getSetting(K_MQTT_PORT, MQTT_PORT).toInt();

    if (_mqtt_user) free(_mqtt_user);
    if (_mqtt_pass) free(_mqtt_pass);
    if (_mqtt_will) free(_mqtt_will);
    if (_mqtt_clientid) free(_mqtt_clientid);

    String user = getSetting(K_MQTT_USER, MQTT_USER);
    _mqttPlaceholders(&user);
    _mqtt_user = strdup(user.c_str());
    _mqtt_pass = strdup(getSetting(K_MQTT_PASS, MQTT_PASS).c_str());
    _mqtt_will = strdup(mqttTopic(MQTT_TOPIC_STATUS, false).c_str());
    String clientid = getSetting(K_MQTT_CLT_ID, getIdentifier());
    _mqttPlaceholders(&clientid);
    _mqtt_clientid = strdup(clientid.c_str());

    DEBUG_MSG_P(PSTR("[MQTT] Connecting to broker at %s:%d\n"), host, port);

    _mqtt_connecting = true;
   
    _mqtt.setServer(host, port);
    _mqtt.setClientId(_mqtt_clientid);
    _mqtt.setKeepAlive(_mqtt_keepalive);
    _mqtt.setCleanSession(false);
    _mqtt.setWill(_mqtt_will, _mqtt_qos, _mqtt_retain, "0");
    if ((strlen(_mqtt_user) > 0) && (strlen(_mqtt_pass) > 0)) {
        DEBUG_MSG_P(PSTR("[MQTT] Connecting as user %s\n"), _mqtt_user);
        _mqtt.setCredentials(_mqtt_user, _mqtt_pass);
    }
   
    #if ASYNC_TCP_SSL_ENABLED
   
        bool secure = getSetting(K_MQTT_USE_SSL, MQTT_SSL_ENABLED).toInt() == 1;
        _mqtt.setSecure(secure);
        if (secure) {
            DEBUG_MSG_P(PSTR("[MQTT] Using SSL\n"));
            unsigned char fp[20] = {0};
            if (sslFingerPrintArray(getSetting(K_MQTT_FP, MQTT_SSL_FINGERPRINT).c_str(), fp)) {
                _mqtt.addServerFingerprint(fp);
            } else {
                DEBUG_MSG_P(PSTR("[MQTT] Wrong fingerprint\n"));
            }
        }
   
    #endif // ASYNC_TCP_SSL_ENABLED
   
    DEBUG_MSG_P(PSTR("[MQTT] Client ID: %s\n"), _mqtt_clientid);
    DEBUG_MSG_P(PSTR("[MQTT] QoS: %d\n"), _mqtt_qos);
    DEBUG_MSG_P(PSTR("[MQTT] Retain flag: %d\n"), _mqtt_retain ? 1 : 0);
    DEBUG_MSG_P(PSTR("[MQTT] Keepalive time: %ds\n"), _mqtt_keepalive);
    DEBUG_MSG_P(PSTR("[MQTT] Will topic: %s\n"), _mqtt_will);
   
    _mqtt.connect();
    free(host);
}

void _mqttPlaceholders(String *text) {
    text->replace("{hostname}", getSetting(K_HOSTNAME));
    text->replace("{magnitude}", "#");
    
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    text->replace("{mac}", mac);
}

void _mqttConfigure() {
    // Get base topic
    _mqtt_topic_base = getSetting(K_MQTT_TOPIC, MQTT_TOPIC);
    if (_mqtt_topic_base.endsWith("/")) _mqtt_topic_base.remove(_mqtt_topic_base.length()-1);

    // Placeholders
    _mqttPlaceholders(&_mqtt_topic_base);
    if (_mqtt_topic_base.indexOf("#") == -1) _mqtt_topic_base = _mqtt_topic_base + "/#";

    // Getters and setters
    _mqtt_setter = getSetting(K_MQTT_SETTER, MQTT_SETTER);
    _mqtt_getter = getSetting(K_MQTT_GETTER, MQTT_GETTER);
    _mqtt_forward = !_mqtt_getter.equals(_mqtt_setter);

    // MQTT options
    _mqtt_qos = getSetting(K_MQTT_QOS, MQTT_QOS).toInt();
    _mqtt_retain = getSetting(K_MQTT_RETAIN, MQTT_RETAIN).toInt() == 1;
    _mqtt_keepalive = getSetting(K_MQTT_KEEP, MQTT_KEEPALIVE).toInt();
    if (getSetting(K_MQTT_CLT_ID).length() == 0) delSetting(K_MQTT_CLT_ID);

    // Enable
    if (getSetting(K_MQTT_SERVER, MQTT_SERVER).length() == 0) {
        mqttEnabled(false);
    } else {
        mqttEnabled(getSetting(K_MQTT_ENABLED, true).toInt() == 1);
    }

    _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;
}

// -----------------------------------------------------------------------------
// MQTT Callbacks
// -----------------------------------------------------------------------------

void _mqttCallback(unsigned int type, const char * topic, const char * payload) {
    if (type == MQTT_CONNECT_EVENT) {
        // Subscribe to internal action topics
        //mqttSubscribe(MQTT_TOPIC_ACTION);
        //mqttSubscribe(MQTT_TOPIC_CONF);

        //char topicToSub[4];
        //snprintf_P(topicToSub, sizeof(topicToSub), PSTR("+/+"));
        mqttSubscribe("+/+");
        //snprintf_P(topicToSub, sizeof(topicToSub), PSTR("+"));
        mqttSubscribe("+");
    }

    if (type == MQTT_MESSAGE_EVENT) {
        // Match topic
        String t = mqttMagnitude((char *) topic);

        // Actions
        if (t.equals(MQTT_TOPIC_ACTION)) {
            if (strcmp(payload, MQTT_ACTION_RESET) == 0) {
                reset();
            }
        }
    }
}

void _mqttOnConnect() {
    DEBUG_MSG_P(PSTR("[MQTT] Connected!\n"));
    _mqtt_reconnect_delay = MQTT_RECONNECT_DELAY_MIN;
    _mqtt_last_connection = millis();

    // Clean subscriptions
    mqttUnsubscribeRaw("#");

    // Send connect event to subscribers
    for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
        (_mqtt_callbacks[i])(MQTT_CONNECT_EVENT, NULL, NULL);
    }
}

void _mqttOnDisconnect() {
    // Reset reconnection delay
    _mqtt_last_connection = millis();
    _mqtt_connecting = false;

    DEBUG_MSG_P(PSTR("[MQTT] Disconnected!\n"));

    // Send disconnect event to subscribers
    for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
        (_mqtt_callbacks[i])(MQTT_DISCONNECT_EVENT, NULL, NULL);
    }
}

void _mqttOnMessage(char* topic, char* payload, unsigned int len) {
    if (len == 0) return;

    char message[len + 1];
    strlcpy(message, (char *) payload, len + 1);

    #if MQTT_SKIP_RETAINED
        if (millis() - _mqtt_last_connection < MQTT_SKIP_TIME) {
            DEBUG_MSG_P(PSTR("[MQTT] Received %s => %s - SKIPPED\n"), topic, message);
			return;
		}
    #endif
    DEBUG_MSG_P(PSTR("[MQTT] Received %s => %s\n"), topic, message);

    // Send message event to subscribers
    for (unsigned char i = 0; i < _mqtt_callbacks.size(); i++) {
        (_mqtt_callbacks[i])(MQTT_MESSAGE_EVENT, topic, message);
    }
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

bool isMqttConnected(){
    return _mqtt.connected();
}

/**
    Returns the magnitude part of a topic

    @param topic the full MQTT topic
    @return String object with the magnitude part.
*/
String mqttMagnitude(char * topic) {
    String pattern = _mqtt_topic_base + _mqtt_setter;
    int position = pattern.indexOf("#");
    if (position == -1) return String();
    String start = pattern.substring(0, position);
    String end = pattern.substring(position + 1);

    String magnitude = String(topic);
    if (magnitude.startsWith(start) && magnitude.endsWith(end)) {
        magnitude.replace(start, "");
        magnitude.replace(end, "");
    } else {
        magnitude = String();
    }

    return magnitude;
}

/**
    Returns a full MQTT topic from the magnitude

    @param magnitude the magnitude part of the topic.
    @param is_set whether to build a command topic (true)
        or a state topic (false).
    @return String full MQTT topic.
*/
String mqttTopic(const char * magnitude, bool is_set) {
    String output = _mqtt_topic_base;
    output.replace("#", magnitude);
    output += is_set ? _mqtt_setter : _mqtt_getter;
    return output;
}

String mqttTopic(const char * magnitude) {
    return mqttTopic(magnitude, false);
}

/**
    Returns a full MQTT topic from the magnitude

    @param magnitude the magnitude part of the topic.
    @param index index of the magnitude when more than one such magnitudes.
    @param is_set whether to build a command topic (true)
        or a state topic (false).
    @return String full MQTT topic.
*/
String mqttTopic(const char * magnitude, unsigned int index, bool is_set) {
    char buffer[strlen(magnitude)+5];
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/%d"), magnitude, index);
    return mqttTopic(buffer, is_set);
}

// -----------------------------------------------------------------------------

void mqttSendRaw(const char * topic, const char * message, bool retain) {
    if (isMqttConnected()) {
        //unsigned int packetId = _mqtt.publish(topic, _mqtt_qos, retain, message);
        //DEBUG_MSG_P(PSTR("[MQTT] Sending %s => %s (PID %d)\n"), topic, message, packetId);
        _mqtt.publish(topic, _mqtt_qos, retain, message);
    }
}

void mqttSendRaw(const char * topic, const char * message) {
    mqttSendRaw (topic, message, _mqtt_retain);
}

void mqttSend(const char * topic, const char * message, bool force, bool retain) {
    mqttSendRaw(mqttTopic(topic, false).c_str(), message, retain);
}

void mqttSend(const char * topic, const char * message, bool force) {
    mqttSend(topic, message, force, _mqtt_retain);
}

void mqttSend(const char * topic, const char * message) {
    mqttSend(topic, message, false);
}

void mqttSend(const char * topic, unsigned int index, const char * message, bool force, bool retain) {
    char buffer[strlen(topic)+5];
    snprintf_P(buffer, sizeof(buffer), PSTR("%s/%d"), topic, index);
    mqttSend(buffer, message, force, retain);
}

void mqttSend(const char * topic, unsigned int index, const char * message, bool force) {
    mqttSend(topic, index, message, force, _mqtt_retain);
}

void mqttSend(const char * topic, unsigned int index, const char * message) {
    mqttSend(topic, index, message, false);
}

void mqttSubscribeRaw(const char * topic) {
    if (isMqttConnected() && (strlen(topic) > 0)) {
        unsigned int packetId = _mqtt.subscribe(topic, _mqtt_qos);
        DEBUG_MSG_P(PSTR("[MQTT] Subscribing to %s (PID %d)\n"), topic, packetId);
    }
}

void mqttSubscribe(const char * topic) {
    mqttSubscribeRaw(mqttTopic(topic, true).c_str());
}

void mqttUnsubscribeRaw(const char * topic) {
    if (isMqttConnected() && (strlen(topic) > 0)) {
        unsigned int packetId = _mqtt.unsubscribe(topic);
        DEBUG_MSG_P(PSTR("[MQTT] Unsubscribing to %s (PID %d)\n"), topic, packetId);
    }
}

void mqttUnsubscribe(const char * topic) {
    mqttUnsubscribeRaw(mqttTopic(topic, true).c_str());
}

// -----------------------------------------------------------------------------

void mqttEnabled(bool status) {
    _mqtt_enabled = status;
}

bool mqttEnabled() {
    return _mqtt_enabled;
}

void mqttDisconnect() {
    if (isMqttConnected()) {
        DEBUG_MSG_P(PSTR("[MQTT] Disconnecting\n"));
        _mqtt.disconnect();
    }
}

void mqttRegister(mqtt_callback_f callback) {
    _mqtt_callbacks.push_back(callback);
}

void saveMqttConfig(String ip, String port, String user, String pass) {
    if(port.equals(""))
        return;

    if(!ip.equals(getSetting(K_MQTT_SERVER, MQTT_SERVER)))
        setSetting(K_MQTT_SERVER, ip);
    if(!ip.equals(getSetting(K_MQTT_PORT, MQTT_PORT)))
        setSetting(K_MQTT_PORT, port);
    if(!ip.equals(getSetting(K_MQTT_USER, MQTT_USER)))
        setSetting(K_MQTT_USER, user);
    if(!ip.equals(getSetting(K_MQTT_PASS, MQTT_PASS)))
        setSetting(K_MQTT_PASS, pass);
}

// -----------------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------------

void mqttSetup() {
     _mqtt.onConnect([](bool sessionPresent) {
         _mqttOnConnect();
     });
     _mqtt.onDisconnect([](AsyncMqttClientDisconnectReason reason) {
         if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
             DEBUG_MSG_P(PSTR("[MQTT] TCP Disconnected\n"));
         }
         if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
             DEBUG_MSG_P(PSTR("[MQTT] Identifier Rejected\n"));
         }
         if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
             DEBUG_MSG_P(PSTR("[MQTT] Server unavailable\n"));
         }
         if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
             DEBUG_MSG_P(PSTR("[MQTT] Malformed credentials\n"));
         }
         if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
             DEBUG_MSG_P(PSTR("[MQTT] Not authorized\n"));
         }
         #if ASYNC_TCP_SSL_ENABLED
         if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
             DEBUG_MSG_P(PSTR("[MQTT] Bad fingerprint\n"));
         }
         #endif
         _mqttOnDisconnect();
     });
     _mqtt.onMessage([](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
		 _mqttOnMessage(topic, payload, len);
		 
     });
     _mqtt.onSubscribe([](uint16_t packetId, uint8_t qos) {
         DEBUG_MSG_P(PSTR("[MQTT] Subscribe ACK for PID %d\n"), packetId);
     });
     _mqtt.onPublish([](uint16_t packetId) {
         DEBUG_MSG_P(PSTR("[MQTT] Publish ACK for PID %d\n"), packetId);
     });

    _mqttConfigure();
    mqttRegister(_mqttCallback);

    // Main callbacks
    espurnaRegisterLoop(mqttLoop);
    espurnaRegisterReload(_mqttConfigure);
}

void mqttLoop() {
    if (!wifiConnected()) return;
	_mqttConnect();
}
