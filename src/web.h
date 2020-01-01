#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include "debug.h"
#include "wifi_s.h"
#include "mqtt.h"
#include "utils.h"


//extern WebServer httpServer(80);


void webServerSetup();
void webServerLoop();
void startWebServer();
void stopWebServer();


#endif