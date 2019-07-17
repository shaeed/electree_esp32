/*

PROTOTYPES HEADER MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include <ArduinoJson.h>

void espurnaRegisterLoop(void (*callback)());
void espurnaRegisterReload(void (*callback)());

//Crate a json document and have it always
#define JSON_CAPACITY      2048  //byte
extern DynamicJsonDocument jsonDoc;

#endif