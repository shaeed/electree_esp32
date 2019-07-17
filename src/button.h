#ifndef BUTTON_H
#define BUTTON_H

#include "DebounceEvent_s.h"
#include <vector>

#include "mqtt.h"
#include "relay.h"
#include "settings.h"
#include "wifi_s.h"
#include "utils.h"

//------------------------------------------------------------------------------
// BUTTONS
//------------------------------------------------------------------------------
#define BUTTON_EVENT_NONE           0
#define BUTTON_EVENT_PRESSED        1
#define BUTTON_EVENT_RELEASED       2
#define BUTTON_EVENT_CLICK          2
#define BUTTON_EVENT_DBLCLICK       3
#define BUTTON_EVENT_LNGCLICK       4
#define BUTTON_EVENT_LNGLNGCLICK    5
#define BUTTON_EVENT_TRIPLECLICK    6

#define BUTTON_MODE_NONE            0
#define BUTTON_MODE_TOGGLE          1
#define BUTTON_MODE_ON              2
#define BUTTON_MODE_OFF             3
#define BUTTON_MODE_AP              4
#define BUTTON_MODE_RESET           5
#define BUTTON_MODE_PULSE           6
#define BUTTON_MODE_FACTORY         7
#define BUTTON_MODE_WPS             8
#define BUTTON_MODE_SMART_CONFIG    9
#define BUTTON_MODE_DIM_UP          10
#define BUTTON_MODE_DIM_DOWN        11

#ifndef BUTTON_DEBOUNCE_DELAY
#define BUTTON_DEBOUNCE_DELAY       50          // Debounce delay (ms)
#endif

#ifndef BUTTON_DBLCLICK_DELAY
#define BUTTON_DBLCLICK_DELAY       500         // Time in ms to wait for a second (or third...) click
#endif

#ifndef BUTTON_LNGCLICK_DELAY
#define BUTTON_LNGCLICK_DELAY       1000        // Time in ms holding the button down to get a long click
#endif

#ifndef BUTTON_LNGLNGCLICK_DELAY
#define BUTTON_LNGLNGCLICK_DELAY    10000       // Time in ms holding the button down to get a long-long click
#endif

#ifndef BUTTON_MQTT_SEND_ALL_EVENTS
#define BUTTON_MQTT_SEND_ALL_EVENTS 0           // 0 - to send only events the are bound to actions
                                                // 1 - to send all button events to MQTT
#endif



void buttonLoop();
void buttonSetup();

#endif