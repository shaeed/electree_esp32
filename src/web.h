#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#if defined(ARDUINO_ARCH_ESP32)
    #include <WebServer.h>
#elif defined(ARDUINO_ARCH_ESP8266)
    #include <ESP8266WebServer.h>
#endif

#include "debug.h"
#include "wifi_s.h"
#include "mqtt.h"
#include "utils.h"


void webServerSetup();
void webServerLoop();
void startWebServer();
void stopWebServer();


#endif