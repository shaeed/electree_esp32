/*

COMMOM DEFINITIONS MODULE

Copyright (C) 2019 by Shaeed Khan

*/

#ifndef DEF_H
#define DEF_H

#define APP_NAME                "ELECTREE"
#define HOSTNAME                ""  //Keep this empty to use the MANUFACTURER and Mac id as host name.
#define MANUFACTURER            "ELECTREE"
#define DEVICE                  "esp01"
#define DEVICE_NAME             MANUFACTURER "_" DEVICE     // Concatenate both to get a unique device name
#define ADMIN_PASS              "electree@1"     // Default password (WEB, OTA, WIFI SoftAP)

#define LOOP_DELAY_TIME         1               // Delay for this millis in the main loop [0-250] (see https://github.com/xoseperez/espurna/issues/1541)

#define JSON_RCONF                 "rcfg"
#define JSON_BCONF                 "bcfg"
#define JSON_LEDCONF               "lcfg"
#define JSON_CONF_RESPONCE         "cres"
#define CONF_MODE                 "m"
#define CONF_GPIO                 "g"
#define CONF_TYPE                 "t"
#define CONF_RELAY_NO             "r"
#define CONF_BUTTON_NO            "b"
#define CONF_RELAY_BOOT_MODE      "bm"
#define CONF_NO_OF_LEDS           "leds"
#define CONF_LED_STRIP_NUMBER     "lno"

#define CONF_MODE_NEW             1
#define CONF_MODE_DELETE_ALL      2
#define CONF_MODE_GET_ALL         3
#define CONF_MODE_GET_SINGLE      4
#define CONF_MODE_UPDATE_SINGLE   5

#endif