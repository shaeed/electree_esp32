/*

MQTT HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef MQTT_H
#define MQTT_H

#include <ArduinoJson.h>
#include <vector>
#include <Ticker.h>

#include <AsyncMqttClient.h>
extern AsyncMqttClient _mqtt;

#include "settings.h"
#include "debug.h"
#include "wifi_s.h"
#include "utils.h"

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------

// Internal MQTT events
#define MQTT_CONNECT_EVENT          0
#define MQTT_DISCONNECT_EVENT       1
#define MQTT_MESSAGE_EVENT          2

#define MQTT_SSL_ENABLED            0               // By default MQTT over SSL will not be enabled
#define MQTT_SSL_FINGERPRINT        ""              // SSL fingerprint of the server
#define MQTT_SERVER                 "192.168.0.100" // Default MQTT broker address
#define MQTT_USER                   ""              // Default MQTT broker usename
#define MQTT_PASS                   ""              // Default MQTT broker password
#define MQTT_PORT                   1883            // MQTT broker port
#define MQTT_TOPIC                  "{hostname}"    // Default MQTT base topic
#define MQTT_RETAIN                 true            // MQTT retain flag
#define MQTT_QOS                    0               // MQTT QoS value for all messages
#define MQTT_KEEPALIVE              300             // MQTT keepalive value

#define MQTT_RECONNECT_DELAY_MIN    5000            // Try to reconnect in 5 seconds upon disconnection
#define MQTT_RECONNECT_DELAY_STEP   5000            // Increase the reconnect delay in 5 seconds after each failed attempt
#define MQTT_RECONNECT_DELAY_MAX    120000          // Set reconnect time to 2 minutes at most

#define MQTT_SKIP_RETAINED          1               // Skip retained messages on connection
#define MQTT_SKIP_TIME              1000            // Skip messages for 1 second anter connection

// These particles will be concatenated to the MQTT_TOPIC base to form the actual topic
#define MQTT_TOPIC_ACTION           "action"
#define MQTT_TOPIC_RELAY            "relay"
#define MQTT_TOPIC_CONF             "conf"
#define MQTT_TOPIC_BUTTON           "button"
#define MQTT_TOPIC_STATUS           "status"
#define MQTT_TOPIC_UARTIN           "uartin"
#define MQTT_TOPIC_UARTOUT          "uartout"
#define MQTT_TOPIC_DEBUG            "debug"
#define MQTT_TOPIC_LEDSTRIP_BRT     "strpbrt"
#define MQTT_TOPIC_LEDSTRIP_THM     "strpthm"
#define MQTT_TOPIC_LEDSTRIP_CLR     "strpclr"
#define MQTT_TOPIC_LEDSTRIP_SPEED   "strpspd"

#define MQTT_STATUS_ONLINE          "1"         // Value for the device ON message
#define MQTT_STATUS_OFFLINE         "0"         // Value for the device OFF message (will)
#define MQTT_ACTION_RESET           "reboot"    // RESET MQTT topic particle

// Custom get and set postfixes
// Use something like "/status" or "/set", with leading slash
#define MQTT_GETTER                 ""
#define MQTT_SETTER                 "/set"

typedef std::function<void(unsigned int, const char *, const char *)> mqtt_callback_f;
void mqttRegister(mqtt_callback_f callback);
String mqttMagnitude(char * topic);

void mqttSetup();
void mqttLoop();
void _mqttPlaceholders(String *text);
String mqttTopic(const char * magnitude, bool is_set) ;
String mqttTopic(const char * magnitude, unsigned int index, bool is_set) ;
void mqttEnabled(bool status);
bool mqttEnabled();
void mqttDisconnect();
void mqttSubscribe(const char * topic);
void mqttSubscribeRaw(const char * topic);
void mqttUnsubscribeRaw(const char * topic);
void mqttUnsubscribe(const char * topic);
void mqttSendRaw(const char * topic, const char * message, bool retain);
void mqttSendRaw(const char * topic, const char * message);
void mqttSend(const char * topic, const char * message, bool force, bool retain);
void mqttSend(const char * topic, const char * message, bool force);
void mqttSend(const char * topic, const char * message);
void mqttSend(const char * topic, unsigned int index, const char * message, bool force, bool retain);
void mqttSend(const char * topic, unsigned int index, const char * message, bool force) ;
void mqttSend(const char * topic, unsigned int index, const char * message);
bool isMqttConnected();
void saveMqttConfig(String ip, String port, String user, String pass);


#endif