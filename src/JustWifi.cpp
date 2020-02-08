/*

JustWifi 2.0.0

Wifi Manager for ESP8266

Copyright (C) 2016-2018 by Xose Pérez <xose dot perez at gmail dot com>

The JustWifi library is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The JustWifi library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the JustWifi library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "JustWifi.h"

//------------------------------------------------------------------------------
// CONSTRUCTOR
//------------------------------------------------------------------------------

JustWifi::JustWifi() {
    _softap.ssid = NULL;
    _timeout = 0;
    uint32_t chip_id;
    #if defined(ARDUINO_ARCH_ESP32)
        chip_id = ESP.getEfuseMac() & 0xFFFFFFFF;
    #else
        chip_id = ESP.getChipId();
    #endif
    snprintf_P(_hostname, sizeof(_hostname), PSTR("ESP_%06X"), chip_id);
}

JustWifi::~JustWifi() {
    cleanNetworks();
}

// -----------------------------------------------------------------------------
// WPS
// -----------------------------------------------------------------------------

#if defined(JUSTWIFI_ENABLE_WPS)
    #if defined(ARDUINO_ARCH_ESP8266)
        wps_cb_status _jw_wps_status;
        
        void _jw_wps_status_cb(wps_cb_status status) {
            _jw_wps_status = status;
        }
    #else
        #include "esp_wps.h"
        esp_wps_config_t _wifi_wps_config = WPS_CONFIG_INIT_DEFAULT(ESP_WPS_MODE);
    #endif

    void JustWifi::startWPS() {
        _state = STATE_WPS_START;
    }
#endif // defined(JUSTWIFI_ENABLE_WPS)

//------------------------------------------------------------------------------
// SMART CONFIG
//------------------------------------------------------------------------------

#if defined(JUSTWIFI_ENABLE_SMARTCONFIG)
    void JustWifi::startSmartConfig() {
        _state = STATE_SMARTCONFIG_START;
    }
#endif // defined(JUSTWIFI_ENABLE_SMARTCONFIG)

//------------------------------------------------------------------------------
// SCAN
//------------------------------------------------------------------------------

uint8_t JustWifi::_sortByRSSI() {
    bool first = true;
    uint8_t bestID = 0xFF;

    for (uint8_t i = 0; i < _network_list.size(); i++) {
        network_t * entry = &_network_list[i];

        // if no data skip
        if (entry->rssi == 0) continue;

        // Empty list
        if (first) {
            first = false;
            bestID = i;
            entry->next = 0xFF;

        // The best so far
        } else if (entry->rssi > _network_list[bestID].rssi) {
            entry->next = bestID;
            bestID = i;

        // Walk the list
        } else {
            network_t * current = &_network_list[bestID];
            while (current->next != 0xFF) {
                if (entry->rssi > _network_list[current->next].rssi) {
                    entry->next = current->next;
                    current->next = i;
                    break;
                }
                current = &_network_list[current->next];
            }

            // Place it the last
            if (current->next == 0xFF) {
                current->next = i;
                entry->next = 0xFF;
            }
        }
    }

    return bestID;
}

String JustWifi::_encodingString(uint8_t security) {
    #if defined(ARDUINO_ARCH_ESP32)
        if (security == WIFI_AUTH_WEP) return String("WEP  ");
        if (security == WIFI_AUTH_WPA_PSK) return String("WPA  ");
        if (security == WIFI_AUTH_WPA2_PSK) return String("WPA2 ");
        if (security == WIFI_AUTH_WPA_WPA2_PSK) return String("WPA* ");
        if (security == WIFI_AUTH_WPA2_ENTERPRISE) return String("WPA2E");
        if (security == WIFI_AUTH_MAX) return String("MAX  ");
    #else
        if (security == ENC_TYPE_WEP) return String("WEP  ");
        if (security == ENC_TYPE_TKIP) return String("WPA  ");
        if (security == ENC_TYPE_CCMP) return String("WPA2 ");
        if (security == ENC_TYPE_AUTO) return String("AUTO ");
    #endif
    return String("OPEN ");
}

uint8_t JustWifi::_populate(uint8_t networkCount) {
    uint8_t count = 0;

    // Reset RSSI to disable networks that have disappeared
    for (uint8_t j = 0; j < _network_list.size(); j++) {
        _network_list[j].rssi = 0;
        _network_list[j].scanned = false;
    }

    String ssid_scan;
    int32_t rssi_scan;
    uint8_t sec_scan;
    uint8_t* BSSID_scan;
    int32_t chan_scan;

    // Populate defined networks with scan data
    for (int8_t i = 0; i < networkCount; ++i) {
        #if defined(ARDUINO_ARCH_ESP32)
            WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan);
        #else
            bool hidden_scan;
            WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan, hidden_scan);
        #endif

        bool known = false;

        for (uint8_t j = 0; j < _network_list.size(); j++) {
            network_t * entry = &_network_list[j];

            if (ssid_scan.equals(entry->ssid)) {
                // Check security
                #if defined(ARDUINO_ARCH_ESP32)
                    if ((sec_scan != WIFI_AUTH_OPEN) && (entry->pass == NULL)) continue;
                #else
                    if ((sec_scan != ENC_TYPE_NONE) && (entry->pass == NULL)) continue;
                #endif

                // In case of several networks with the same SSID
                // we want to get the one with the best RSSI
                // Thanks to Robert (robi772 @ bitbucket.org)
                if (entry->rssi < rssi_scan || entry->rssi == 0) {
                    entry->rssi = rssi_scan;
                    entry->security = sec_scan;
                    entry->channel = chan_scan;
                    entry->scanned = true;
                    memcpy((void*) &entry->bssid, (void*) BSSID_scan, sizeof(entry->bssid));
                }

                count++;
                known = true;
                break;
            }
        }

        if (_callbacks.size()) {
            char buffer[128];
            sprintf_P(buffer,
                PSTR("%s BSSID: %02X:%02X:%02X:%02X:%02X:%02X CH: %2d RSSI: %3d SEC: %s SSID: %s"),
                (known ? "-->" : "   "),
                BSSID_scan[0], BSSID_scan[1], BSSID_scan[2], BSSID_scan[3], BSSID_scan[4], BSSID_scan[5],
                chan_scan,
                rssi_scan,
                _encodingString(sec_scan).c_str(),
                ssid_scan.c_str()
            );
            _doCallback(MESSAGE_FOUND_NETWORK, buffer);
        }
    }

    return count;
}

uint8_t JustWifi::_doScan() {
    static bool scanning = false;

    // If not scanning, start scan
    if (false == scanning) {
        WiFi.disconnect();
        WiFi.enableSTA(true);
        WiFi.scanNetworks(true, true);
        _doCallback(MESSAGE_SCANNING);
        scanning = true;
        return RESPONSE_WAIT;
    }

    // Check if scanning
    int8_t scanResult = WiFi.scanComplete();
    if (WIFI_SCAN_RUNNING == scanResult) {
        return RESPONSE_WAIT;
    }

    // Scan finished
    scanning = false;

    // Sometimes the scan fails,
    // this will force the scan to restart
    if (WIFI_SCAN_FAILED == scanResult) {
        _doCallback(MESSAGE_SCAN_FAILED);
        return RESPONSE_WAIT;
    }

    // Check networks
    if (0 == scanResult) {
        //WiFi.enableSTA(false); Commented by me as JustWifi for esp8266 is not having this.
        _doCallback(MESSAGE_NO_NETWORKS);
        return RESPONSE_FAIL;
    }

    // Populate network list
    uint8_t count = _populate(scanResult);

    // Free memory
    WiFi.scanDelete();

    if (0 == count) {
        //WiFi.enableSTA(false); Commented by me as JustWifi for esp8266 is not having this.
        _doCallback(MESSAGE_NO_KNOWN_NETWORKS);
        return RESPONSE_FAIL;
    }

    // Sort networks by RSSI
    _currentID = _sortByRSSI();
    return RESPONSE_OK;
}

// -----------------------------------------------------------------------------
// STA
// -----------------------------------------------------------------------------

uint8_t JustWifi::_doSTA(uint8_t id) {
    static uint8_t networkID;
    static uint8_t state = RESPONSE_START;
    static unsigned long timeout;

    // Reset connection process
    if (id != 0xFF) {
        state = RESPONSE_START;
        networkID = id;
    }

    // Get network
    network_t entry = _network_list[networkID];

    // No state or previous network failed
    if (RESPONSE_START == state) {
        // Init some values
        WiFi.persistent(false);
        WiFi.enableSTA(true);
        
        #if defined(ARDUINO_ARCH_ESP32) //Added by me as JustWifi for esp8266 is having this.
            WiFi.setHostname(_hostname);
        #else
            WiFi.hostname(_hostname);
        #endif

        // Configure static options
        if (!entry.dhcp) {
            WiFi.config(entry.ip, entry.gw, entry.netmask, entry.dns);
        }

        // Info callback
        if (_callbacks.size()) {
            char buffer[128];
            if (entry.scanned) {
                snprintf_P(buffer, sizeof(buffer),
                    PSTR("BSSID: %02X:%02X:%02X:%02X:%02X:%02X CH: %02d, RSSI: %3d, SEC: %s, SSID: %s"),
                    entry.bssid[0], entry.bssid[1], entry.bssid[2], entry.bssid[3], entry.bssid[4], entry.bssid[5],
                    entry.channel,
                    entry.rssi,
                    _encodingString(entry.security).c_str(),
                    entry.ssid
                );
            } else {
                snprintf_P(buffer, sizeof(buffer), PSTR("SSID: %s"), entry.ssid);
            }
            _doCallback(MESSAGE_CONNECTING, buffer);
        }

        // Connecting
        if (entry.channel == 0) {
            WiFi.begin(entry.ssid, entry.pass);
        } else {
            WiFi.begin(entry.ssid, entry.pass, entry.channel, entry.bssid);
        }

        timeout = millis();
        return (state = RESPONSE_WAIT);
    }

    // Connected?
    if (WiFi.status() == WL_CONNECTED) {
        // Hostname
        #if defined(ARDUINO_ARCH_ESP32)
            WiFi.setHostname(_hostname);
        #else
            WiFi.hostname(_hostname);
        #endif

        // Autoconnect only if DHCP, since it doesn't store static IP data
        WiFi.setAutoConnect(entry.dhcp);

        WiFi.setAutoReconnect(true);
        _doCallback(MESSAGE_CONNECTED);

        return (state = RESPONSE_OK);
    }

    // Check timeout
    if (millis() - timeout > _connect_timeout) {
        WiFi.enableSTA(false);
        _doCallback(MESSAGE_CONNECT_FAILED, entry.ssid);
        return (state = RESPONSE_FAIL);
    }

    // Still waiting
    _doCallback(MESSAGE_CONNECT_WAITING);
    return state;

}

// -----------------------------------------------------------------------------
// AP
// -----------------------------------------------------------------------------

bool JustWifi::_doAP() {
    // If already created recreate
    if (_ap_connected) {
        enableAP(false);
        delay(2);
    }

    // Check if Soft AP configuration defined
    if (!_softap.ssid) {
        _softap.ssid = strdup(_hostname);
    }

    _doCallback(MESSAGE_ACCESSPOINT_CREATING);
    WiFi.enableAP(true);

    // Configure static options
    if (_softap.dhcp) {
        WiFi.softAPConfig(_softap.ip, _softap.gw, _softap.netmask);
    }

    if (_softap.pass) {
        WiFi.softAP(_softap.ssid, _softap.pass);
    } else {
        WiFi.softAP(_softap.ssid);
    }
    _doCallback(MESSAGE_ACCESSPOINT_CREATED);

    _ap_connected = true;
    return true;

}

// -----------------------------------------------------------------------------
// CALLBACKS
// -----------------------------------------------------------------------------

void JustWifi::_doCallback(justwifi_messages_t message, char * parameter) {
    for (unsigned char i=0; i < _callbacks.size(); i++) {
        (_callbacks[i])(message, parameter);
    }
}

void JustWifi::subscribe(TMessageFunction fn) {
    _callbacks.push_back(fn);
}

// -----------------------------------------------------------------------------
// STATE MACHINE
// -----------------------------------------------------------------------------

void JustWifi::_machine() {
    switch(_state) {
        case STATE_IDLE:
            if (WiFi.status() == WL_CONNECTED)
                return;

            // Should we connect in STA mode?
            if (_sta_enabled && _network_list.size() > 0) {
                if ((0 == _timeout) || ((_reconnect_timeout > 0) && (millis() - _timeout > _reconnect_timeout))) {
                    _currentID = 0;
                    _state = _scan ? STATE_SCAN_START : STATE_STA_START;
                    return;
                }
            }
            // Fallback
            if (!_ap_connected && _ap_fallback_enabled) {
                _state = STATE_FALLBACK;
            }
            break;

        case STATE_SCAN_START:
            _doScan();
            _state = STATE_SCAN_ONGOING;
            break;

        case STATE_SCAN_ONGOING:
            {
                uint8_t response = _doScan();
                if (RESPONSE_OK == response) {
                    _state = STATE_STA_START;
                } else if (RESPONSE_FAIL == response) {
                    _state = STATE_FALLBACK;
                }
            }
            break;

        case STATE_STA_START:
            _doSTA(_currentID);
            _state = STATE_STA_ONGOING;
            break;

        case STATE_STA_ONGOING:
            {
                uint8_t response = _doSTA();
                if (RESPONSE_OK == response) {
                    _state = STATE_STA_SUCCESS;
                } else if (RESPONSE_FAIL == response) {
                    _state = STATE_STA_START;
                    if (_scan) {
                        _currentID = _network_list[_currentID].next;
                        if (_currentID == 0xFF) {
                            _state = STATE_STA_FAILED;
                        }
                    } else {
                        _currentID++;
                        if (_currentID == _network_list.size()) {
                            _state = STATE_STA_FAILED;
                        }
                    }
                }
            }
            break;

        case STATE_STA_FAILED:
            _state = STATE_FALLBACK;
            break;

        case STATE_STA_SUCCESS:
            _state = STATE_IDLE;
            break;

        #if defined(JUSTWIFI_ENABLE_WPS)
        case STATE_WPS_START:
            _doCallback(MESSAGE_WPS_START);
            #if defined(ARDUINO_ARCH_ESP8266)
                _esp8266_153_reset();

                if (!WiFi.enableSTA(true)) {
                    _state = STATE_WPS_FAILED;
                    return;
                }

                WiFi.disconnect();
                if (!wifi_wps_disable()) {
                    _state = STATE_WPS_FAILED;
                    return;
                }

                // so far only WPS_TYPE_PBC is supported (SDK 1.2.0)
                if (!wifi_wps_enable(WPS_TYPE_PBC)) {
                    _state = STATE_WPS_FAILED;
                    return;
                }

                _jw_wps_status = (wps_cb_status) 5;
                if (!wifi_set_wps_cb((wps_st_cb_t) &_jw_wps_status_cb)) {
                    _state = STATE_WPS_FAILED;
                    return;
                }

                if (!wifi_wps_start()) {
                    _state = STATE_WPS_FAILED;
                    return;
                }
            #else
                esp_wifi_wps_enable(&_wifi_wps_config);
                esp_wifi_wps_start(0);
            #endif

            _state = STATE_WPS_ONGOING;
            break;

        case STATE_WPS_ONGOING:
            if (5 == _jw_wps_status) {
                // Still ongoing
            } else if (WPS_CB_ST_SUCCESS == _jw_wps_status) {
                _state = STATE_WPS_SUCCESS;
            } else {
                _state = STATE_WPS_FAILED;
            }
            break;

        case STATE_WPS_FAILED:
            _doCallback(MESSAGE_WPS_ERROR);
            wifi_wps_disable();
            _state = STATE_FALLBACK;
            break;

        case STATE_WPS_SUCCESS:
            _doCallback(MESSAGE_WPS_SUCCESS);
            wifi_wps_disable();
            addCurrentNetwork(true);
            _state = STATE_IDLE;
            break;
        #endif // defined(JUSTWIFI_ENABLE_WPS)

        #if defined(JUSTWIFI_ENABLE_SMARTCONFIG)
        case STATE_SMARTCONFIG_START:
            _doCallback(MESSAGE_SMARTCONFIG_START);
            enableAP(false);

            if (!WiFi.beginSmartConfig()) {
                _state = STATE_SMARTCONFIG_FAILED;
                return;
            }

            _state = STATE_SMARTCONFIG_ONGOING;
            _start = millis();
            break;

        case STATE_SMARTCONFIG_ONGOING:
            if (WiFi.smartConfigDone()) {
                _state = STATE_SMARTCONFIG_SUCCESS;
            } else if (millis() - _start > JUSTWIFI_SMARTCONFIG_TIMEOUT) {
                _state = STATE_SMARTCONFIG_FAILED;
            }
            break;

        case STATE_SMARTCONFIG_FAILED:
            _doCallback(MESSAGE_SMARTCONFIG_ERROR);
            WiFi.stopSmartConfig();
            WiFi.enableSTA(false);
            _state = STATE_FALLBACK;
            break;

        case STATE_SMARTCONFIG_SUCCESS:
            _doCallback(MESSAGE_SMARTCONFIG_SUCCESS);
            addCurrentNetwork(true);
            _state = STATE_IDLE;
            break;
        #endif // defined(JUSTWIFI_ENABLE_SMARTCONFIG)

        case STATE_FALLBACK:
            if (!_ap_connected && _ap_fallback_enabled) _doAP();
            _timeout = millis();
            _state = STATE_IDLE;
            break;

        default:
            _state = STATE_IDLE;
            break;
    }
}

//------------------------------------------------------------------------------
// PUBLIC API
//------------------------------------------------------------------------------

void JustWifi::cleanNetworks() {
    for (uint8_t i = 0; i < _network_list.size(); i++) {
        network_t entry = _network_list[i];
        if (entry.ssid) free(entry.ssid);
        if (entry.pass) free(entry.pass);
    }
    _network_list.clear();
}

bool JustWifi::addNetwork(
    const char * ssid,
    const char * pass,
    const char * ip,
    const char * gw,
    const char * netmask,
    const char * dns,
    bool front
) {

    network_t new_network;

    // Check SSID too long or missing
    if (!ssid || *ssid == 0x00 || strlen(ssid) > 31) {
        return false;
    }

    // Check PASS too long
    if (pass && strlen(pass) > 63) {
        return false;
    }

    // Copy network SSID
    new_network.ssid = strdup(ssid);
    if (!new_network.ssid) {
        return false;
    }

    // Copy network PASS
    if (pass && *pass != 0x00) {
        new_network.pass = strdup(pass);
        if (!new_network.pass) {
            free(new_network.ssid);
            return false;
        }
    } else {
        new_network.pass = NULL;
    }

    // Copy static config
    new_network.dhcp = true;
    if (ip && gw && netmask
        && *ip != 0x00 && *gw != 0x00 && *netmask != 0x00) {
        new_network.dhcp = false;
        new_network.ip.fromString(ip);
        new_network.gw.fromString(gw);
        new_network.netmask.fromString(netmask);
    }
    if (dns && *dns != 0x00) {
        new_network.dns.fromString(dns);
    }

    // Defaults
    new_network.rssi = 0;
    new_network.security = 0;
    new_network.channel = 0;
    new_network.next = 0xFF;
    new_network.scanned = false;

    // Store data
    if (front) {
        _network_list.insert(_network_list.begin(), new_network);
    } else {
        _network_list.push_back(new_network);
    }
    return true;

}

bool JustWifi::addCurrentNetwork(bool front) {
    return addNetwork(
        WiFi.SSID().c_str(),
        WiFi.psk().c_str(),
        NULL, NULL, NULL, NULL,
        front
    );
}

bool JustWifi::setSoftAP(
    const char * ssid,
    const char * pass,
    const char * ip,
    const char * gw,
    const char * netmask
) {

    // Check SSID too long or missing
    if (!ssid || *ssid == 0x00 || strlen(ssid) > 31) {
        return false;
    }

    // Check PASS too long
    if (pass && strlen(pass) > 63) {
        return false;
    }

    // Copy network SSID
    _softap.ssid = strdup(ssid);
    if (!_softap.ssid) {
        return false;
    }

    // Copy network PASS
    if (pass && *pass != 0x00) {
        _softap.pass = strdup(pass);
        if (!_softap.pass) {
            _softap.ssid = NULL;
            return false;
        }
    }

    // Copy static config
    _softap.dhcp = false;
    if (ip && gw && netmask
        && *ip != 0x00 && *gw != 0x00 && *netmask != 0x00) {
        _softap.dhcp = true;
        _softap.ip.fromString(ip);
        _softap.gw.fromString(gw);
        _softap.netmask.fromString(netmask);
    }

    if ((WiFi.getMode() & WIFI_AP) > 0) {

    // https://github.com/xoseperez/justwifi/issues/4
    if (_softap.pass) {
        WiFi.softAP(_softap.ssid, _softap.pass);
    } else {
        WiFi.softAP(_softap.ssid);
        }

    }

    return true;

}

void JustWifi::setConnectTimeout(unsigned long ms) {
    _connect_timeout = ms;
}

void JustWifi::setReconnectTimeout(unsigned long ms) {
    _reconnect_timeout = ms;
}

void JustWifi::resetReconnectTimeout() {
    _timeout = millis();
}

void JustWifi::setHostname(const char * hostname) {
    strncpy(_hostname, hostname, sizeof(_hostname));
}

String JustWifi::getAPSSID() {
    return String(_softap.ssid);
}

bool JustWifi::connectable() {
    return _ap_connected;
}

void JustWifi::disconnect() {
    _timeout = 0;
    WiFi.disconnect();
    WiFi.enableSTA(false);
    _doCallback(MESSAGE_DISCONNECTED);
}

void JustWifi::turnOff() {

    WiFi.disconnect();
    WiFi.enableAP(false);
    WiFi.enableSTA(false);
    #if defined(ARDUINO_ARCH_ESP8266)
        WiFi.forceSleepBegin();
    #endif
    delay(1);

    _doCallback(MESSAGE_TURNING_OFF);
    _sta_enabled = false;
    _state = STATE_IDLE;

}

void JustWifi::turnOn() {

    #if defined(ARDUINO_ARCH_ESP8266)
        WiFi.forceSleepWake();
    #endif
    delay(1);

    setReconnectTimeout(0);
    _doCallback(MESSAGE_TURNING_ON);

    //#if defined(ARDUINO_ARCH_ESP8266) //Added by me from esp8266 code
        WiFi.enableSTA(true);
    //#endif
    _sta_enabled = true;
    _state = STATE_IDLE;

}

void JustWifi::enableSTA(bool enabled) {
    _sta_enabled = enabled;
}

/*void JustWifi::enableAP(bool enabled) {
    if (enabled) {
        _doAP();
    //} else if (_ap_connected) { //Deleted the else if condition by me as esp8266 code
    } else
    #if defined(ARDUINO_ARCH_ESP32)
        if (_ap_connected)
    #endif
    {
        WiFi.softAPdisconnect();
        WiFi.enableAP(false);
        _ap_connected = false;
        _doCallback(MESSAGE_ACCESSPOINT_DESTROYED);
    }
}*/

void JustWifi::enableAP(bool enabled) {
    if (enabled) {
        _doAP();
    //} else if (_ap_connected) { //Deleted the else if condition by me as esp8266 code
    } else 
	#if defined(ARDUINO_ARCH_ESP32)
		if (_ap_connected)
	#endif
	{
        WiFi.softAPdisconnect();
        WiFi.enableAP(false);
        _ap_connected = false;
        _doCallback(MESSAGE_ACCESSPOINT_DESTROYED);
    }
}

void JustWifi::enableAPFallback(bool enabled) {
    _ap_fallback_enabled = enabled;
}

void JustWifi::enableScan(bool scan) {
    _scan = scan;
}

void JustWifi::_events(WiFiEvent_t event) {
    Serial.printf("[WIFI] Event %u\n", (uint8_t) event);
}

void JustWifi::init() {
    WiFi.enableAP(false);
    WiFi.enableSTA(false);
    //WiFi.onEvent(reinterpret_cast<WiFiEventCb>(&JustWifi::_events), WIFI_EVENT_ANY);
}

void JustWifi::loop() {
    _machine();
}

JustWifi jw;
