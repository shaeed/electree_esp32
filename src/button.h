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
//#ifndef BUTTON_PUSHBUTTON
#define BUTTON_PUSHBUTTON           0
#define BUTTON_SWITCH               1
#define BUTTON_DEFAULT_HIGH         2
#define BUTTON_SET_PULLUP           4
//#endif

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

/////Note: millis() function is not working for esp32. Below time are multipled by 10.
#define BUTTON_DEBOUNCE_DELAY       50          // Debounce delay (ms)
#define BUTTON_DBLCLICK_DELAY       500         // Time in ms to wait for a second (or third...) click
#define BUTTON_LNGCLICK_DELAY       1000        // Time in ms holding the button down to get a long click
#define BUTTON_LNGLNGCLICK_DELAY    10000       // Time in ms holding the button down to get a long-long click

#define BUTTON_MQTT_SEND_ALL_EVENTS 1           // 0 - to send only events the are bound to actions
                                                // 1 - to send all button events to MQTT



void buttonLoop();
void buttonSetup();

#endif