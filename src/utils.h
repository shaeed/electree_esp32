/*

UTILS HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "wifi_s.h"
#include "debug.h"
#include "def.h"
#include "settings.h"

#include <Ticker.h>
extern Ticker _defer_reset;

bool isNumber(const char * s);
void nice_delay(unsigned long ms);
double roundTo(double num, unsigned char positions);
char * ltrim(char * s);
void reset();
void info();
void setDefaultHostname();
String getIdentifier();
String getAdminPass();
String getBoardName() ;

#endif