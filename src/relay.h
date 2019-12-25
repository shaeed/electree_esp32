#ifndef RELAY_H
#define RELAY_H

#include <EEPROM.h>
//#include <Ticker.h>
#include <ArduinoJson.h>
#include <vector>
//#include <functional>
#include "prototypes.h"
#include "settings.h"
#include "debug.h"
#include "utils.h"

#define GPIO_NONE           0x99

#define RELAY_BOOT_OFF              0
#define RELAY_BOOT_ON               1
#define RELAY_BOOT_SAME             2
#define RELAY_BOOT_TOGGLE           3

#define RELAY_TYPE_NORMAL           0
#define RELAY_TYPE_INVERSE          1
#define RELAY_TYPE_PWM              2

// Default boot mode: 0 means OFF, 1 ON and 2 whatever was before
#ifndef RELAY_BOOT_MODE
#define RELAY_BOOT_MODE             RELAY_BOOT_SAME
#endif

// Relay requests flood protection window - in seconds
#ifndef RELAY_FLOOD_WINDOW
#define RELAY_FLOOD_WINDOW          3
#endif

// Allowed actual relay changes inside requests flood protection window
#ifndef RELAY_FLOOD_CHANGES
#define RELAY_FLOOD_CHANGES         5
#endif

// Do not save relay state after these many milliseconds
#ifndef RELAY_SAVE_DELAY
#define RELAY_SAVE_DELAY            1000
#endif

// Configure the MQTT payload for ON/OFF
#ifndef RELAY_MQTT_ON
#define RELAY_MQTT_ON               "1"
#endif
#ifndef RELAY_MQTT_OFF
#define RELAY_MQTT_OFF              "0"
#endif

void _relayProviderStatus(unsigned char id, bool status);
void _relayProcess(bool mode);
void relayPulse(unsigned char id);
bool relayStatus(unsigned char id, bool status, bool report, bool group_report);
bool relayStatus(unsigned char id, bool status);
bool relayStatus(unsigned char id);
void relaySync(unsigned char id);
void relaySave(bool do_commit);
void relaySave();
void relayToggle(unsigned char id, bool report, bool group_report);
void relayToggle(unsigned char id);
unsigned char relayCount();
unsigned char relayParsePayload(const char * payload);
void _relayBoot();
void _relayConfigure();
void _relayMQTTGroup(unsigned char id);
void publishRelayStatus(unsigned char id);
void publishRelayStatus();
//void relayStatusWrap(unsigned char id, unsigned char value, bool is_group_topic);
void relayMQTTCallback(unsigned int type, const char * topic, const char * payload);
void relaySetupMQTT();
void _relayLoop();
void relaySetup();

#endif