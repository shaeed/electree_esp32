/*

WIFI HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef WIFI_S_H
#define WIFI_S_H

#include <Ticker.h>
#include "JustWifi.h"
#include "debug.h"
#include "prototypes.h"
#include "def.h"
#include "settings.h"

#define WIFI_STATE_AP               1
#define WIFI_STATE_STA              2
#define WIFI_STATE_AP_STA           3
#define WIFI_STATE_WPS              4
#define WIFI_STATE_SMARTCONFIG      8

#define WIFI_AP_ALLWAYS             1
#define WIFI_AP_FALLBACK            2

// -----------------------------------------------------------------------------
// WIFI
// -----------------------------------------------------------------------------

#define WIFI_CONNECT_TIMEOUT        60000               // Connecting timeout for WIFI in ms
#define WIFI_RECONNECT_INTERVAL     180000              // If could not connect to WIFI, retry after this time in ms
#define WIFI_MAX_NETWORKS           5                   // Max number of WIFI connection configurations
#define WIFI_FALLBACK_APMODE        1                   // Fallback to AP mode if no STA connection
#define WIFI_SLEEP_MODE             WIFI_NONE_SLEEP     // WIFI_NONE_SLEEP, WIFI_LIGHT_SLEEP or WIFI_MODEM_SLEEP
#define WIFI_SCAN_NETWORKS          1                   // Perform a network scan before connecting

// Optional hardcoded configuration (up to 2 networks)
#define WIFI1_SSID                  "AKG"
#define WIFI1_PASS                  "Ashwani#$458941"
#define WIFI1_IP                    ""
#define WIFI1_GW                    ""
#define WIFI1_MASK                  ""
#define WIFI1_DNS                   ""

#define WIFI2_SSID                  ""
#define WIFI2_PASS                  ""
#define WIFI2_IP                    ""
#define WIFI2_GW                    ""
#define WIFI2_MASK                  ""
#define WIFI2_DNS                   ""

#ifndef JUSTWIFI_ENABLE_SMARTCONFIG
#define JUSTWIFI_ENABLE_SMARTCONFIG
#endif

typedef std::function<void(justwifi_messages_t code, char * parameter)> wifi_callback_f;
void wifiRegister(wifi_callback_f callback);
bool wifiConnected();

void wifiSetup();
void wifiLoop();
void wifiReconnectCheck();
bool _wifiClean(unsigned char num);
void wifiDebug(WiFiMode_t modes);
void wifiRegister(wifi_callback_f callback);
String getIP();
void wifiStartAP(bool only);
void wifiStartAP();
bool wifiConnected();

#endif