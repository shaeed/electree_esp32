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

#ifndef MQTT_SSL_ENABLED
#define MQTT_SSL_ENABLED            0               // By default MQTT over SSL will not be enabled
#endif

#ifndef MQTT_SSL_FINGERPRINT
#define MQTT_SSL_FINGERPRINT        ""              // SSL fingerprint of the server
#endif

#ifndef MQTT_SERVER
#define MQTT_SERVER                 "192.168.0.108"              // Default MQTT broker address
#endif

#ifndef MQTT_USER
#define MQTT_USER                   ""              // Default MQTT broker usename
#endif

#ifndef MQTT_PASS
#define MQTT_PASS                   ""              // Default MQTT broker password
#endif

#ifndef MQTT_PORT
#define MQTT_PORT                   1883            // MQTT broker port
#endif

#ifndef MQTT_TOPIC
#define MQTT_TOPIC                  "{hostname}"    // Default MQTT base topic
#endif

#ifndef MQTT_RETAIN
#define MQTT_RETAIN                 true            // MQTT retain flag
#endif

#ifndef MQTT_QOS
#define MQTT_QOS                    0               // MQTT QoS value for all messages
#endif

#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE              300             // MQTT keepalive value
#endif


#ifndef MQTT_RECONNECT_DELAY_MIN
#define MQTT_RECONNECT_DELAY_MIN    5000            // Try to reconnect in 5 seconds upon disconnection
#endif

#ifndef MQTT_RECONNECT_DELAY_STEP
#define MQTT_RECONNECT_DELAY_STEP   5000            // Increase the reconnect delay in 5 seconds after each failed attempt
#endif

#ifndef MQTT_RECONNECT_DELAY_MAX
#define MQTT_RECONNECT_DELAY_MAX    120000          // Set reconnect time to 2 minutes at most
#endif


#ifndef MQTT_SKIP_RETAINED
#define MQTT_SKIP_RETAINED          1               // Skip retained messages on connection
#endif

#ifndef MQTT_SKIP_TIME
#define MQTT_SKIP_TIME              1000            // Skip messages for 1 second anter connection
#endif


#ifndef MQTT_USE_JSON
#define MQTT_USE_JSON               0               // Group messages in a JSON body
#endif

#ifndef MQTT_USE_JSON_DELAY
#define MQTT_USE_JSON_DELAY         100             // Wait this many ms before grouping messages
#endif

#ifndef MQTT_QUEUE_MAX_SIZE
#define MQTT_QUEUE_MAX_SIZE         20              // Size of the MQTT queue when MQTT_USE_JSON is enabled
#endif

// These particles will be concatenated to the MQTT_TOPIC base to form the actual topic
#define MQTT_TOPIC_JSON             "data"
#define MQTT_TOPIC_ACTION           "action"
#define MQTT_TOPIC_RELAY            "relay"
#define MQTT_TOPIC_CONF             "conf"
#define MQTT_TOPIC_BUTTON           "button"
#define MQTT_TOPIC_IP               "ip"
#define MQTT_TOPIC_STATUS           "status"
#define MQTT_TOPIC_MAC              "mac"
#define MQTT_TOPIC_HOSTNAME         "host"
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
#ifndef MQTT_GETTER
#define MQTT_GETTER                 ""
#endif

#ifndef MQTT_SETTER
#define MQTT_SETTER                 "/set"
#endif

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


#endif