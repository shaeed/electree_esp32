#include "led2812b.h"

std::vector<Led2812b*> _strips;

Led2812b::Led2812b(uint16_t noOfLeds, uint8_t ledGpio){
    _leds = new CRGB[noOfLeds];
    _hue = new uint8_t[noOfLeds];
    _noOfLeds = noOfLeds;
    pinMode(ledGpio, OUTPUT);
    _setupFastLed(noOfLeds, ledGpio);
}

Led2812b::~Led2812b(){
    delete[] _leds;
    delete[] _hue;
}

void Led2812b::loop(){
    
    if(_applyBrightness) {
        FastLED.setBrightness(_brt);
        _applyBrightness = false;
        FastLED.show();
    }

    if(_applyClr){
        applyColor(_clr);
        Serial.println("in apply color");
        _applyClr = false;
        FastLED.show();
    }

    if(!_repeat)
        return;

    switch (_theme)
    {
    case THEME_OFF:
        applyColor(CRGB::Black);
        _repeat = false;
        _updateLeds = true;
        break;

    case THEME_RAINBOW:
        rainbowEffect();
        break;

    case THEME_ALLWHITE:
        applyColor(CRGB::White);
        _repeat = false;
        _updateLeds = true;
        break;

    case THEME_COLORFUL:
        applyColor(_clr);
        _repeat = false;
        _applyClr = false;
        _updateLeds = true;
        break;

    case THEME_FADE:
        simpleFade();
        break;

    case THEME_NIGHTSKY:
        NightSky();
        break;

    case Theme_RGBLoop:
        RGBLoop();
        break;

    case Theme_theaterChaseRainbow:
      theaterChaseRainbow();
    break;

    case Theme_HueEffect:
      HueEffect();
    break;

    default:
        break;
    }

    if(_updateLeds) {
        FastLED.show();
        _updateLeds = false;
    }
}

void Led2812b::_setupFastLed(uint16_t noOfLeds, uint8_t gpio) {
    switch (gpio) {
    case 0:
        FastLED.addLeds<STRIP_TYPE, 0, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 1:
        FastLED.addLeds<STRIP_TYPE, 1, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 2:
        FastLED.addLeds<STRIP_TYPE, 2, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 3:
        FastLED.addLeds<STRIP_TYPE, 3, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 4:
        FastLED.addLeds<STRIP_TYPE, 4, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 5:
        FastLED.addLeds<STRIP_TYPE, 5, COLOR_MODE>(_leds, noOfLeds);
        break;
#if defined(ARDUINO_ARCH_ESP32)
    case 12:
        FastLED.addLeds<STRIP_TYPE, 12, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 13:
        FastLED.addLeds<STRIP_TYPE, 13, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 14:
        FastLED.addLeds<STRIP_TYPE, 14, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 15:
        FastLED.addLeds<STRIP_TYPE, 15, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 16:
        FastLED.addLeds<STRIP_TYPE, 16, COLOR_MODE>(_leds, noOfLeds);
        break;
    

    case 17:
        FastLED.addLeds<STRIP_TYPE, 17, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 18:
        FastLED.addLeds<STRIP_TYPE, 18, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 19:
        FastLED.addLeds<STRIP_TYPE, 19, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 21:
        FastLED.addLeds<STRIP_TYPE, 21, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 22:
        FastLED.addLeds<STRIP_TYPE, 22, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 23:
        FastLED.addLeds<STRIP_TYPE, 23, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 25:
        FastLED.addLeds<STRIP_TYPE, 25, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 26:
        FastLED.addLeds<STRIP_TYPE, 26, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 27:
        FastLED.addLeds<STRIP_TYPE, 27, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 32:
        FastLED.addLeds<STRIP_TYPE, 32, COLOR_MODE>(_leds, noOfLeds);
        break;
    case 33:
        FastLED.addLeds<STRIP_TYPE, 33, COLOR_MODE>(_leds, noOfLeds);
        break;
#endif
    default:
        break;
    }
}

void Led2812b::_initVars(){
    //applyColor(CRGB::Black);
    _brtTemp = 0;
    _showBlack = true;
    _hueInt = 0;
    if(_brt < 10)
        _brt = 10;

    if(_theme == THEME_FADE){
        _applyClr = true;
    } else if(_theme == THEME_NIGHTSKY){
        _clr = CRGB::White;
    }
}

void Led2812b::setPixel(int Pixel, byte red, byte green, byte blue) {
  _leds[Pixel].r = red;
  _leds[Pixel].g = green;
  _leds[Pixel].b = blue;
}
void Led2812b::setPixel(int Pixel, CRGB clr) {
  _leds[Pixel] = clr;
}

void Led2812b::setAll(byte red, byte green, byte blue) {
    CRGB clr;
    clr.r = red;
    clr.b = blue;
    clr.g = green;
    applyColor(clr);
}

void Led2812b::applyColor(CRGB clr){
  fill_solid(_leds, _noOfLeds, clr);
  delay(DELAY_APPLY_COLOR_ALL);
}

void Led2812b::HueEffect() {
    if(_delayTrue && millis()-_waitStartMillis < _spd * THEME_HUEEFFECT_MULTIPLIER) {
        return;
    }

    for (int i = 0; i < _noOfLeds; i++) {
        _leds[i] = CHSV(_hue[i]++, 255, 255);
    }

    _updateLeds = true;
    _delayTrue = true;
    _waitStartMillis = millis();
}

void Led2812b::rainbowEffect(){
    if(millis()-_waitStartMillis < _spd * THEME_RAINBOW_MULTIPLIER) {
        return;
    }

    fill_rainbow(_leds, _noOfLeds, _hueInt, 5);
    _hueInt += 15;

    _updateLeds = true;
    _waitStartMillis = millis();
}

void Led2812b::simpleFade(){
    if(_delayTrue && millis()-_waitStartMillis < _spd * THEME_FADE_MULTIPLIER) {
        return;
    }

    if(_showBlack){
        //Fade in
        FastLED.setBrightness(_brtTemp);
        _brtTemp++;
        if(_brtTemp >= _brt)
          _showBlack = false;
      } else {
        //Fade out
        FastLED.setBrightness(_brtTemp);
        _brtTemp--;
        if(_brtTemp < 1)
          _showBlack = true;
    }

    _updateLeds = true;
    _delayTrue = true;
    _waitStartMillis = millis();
}

void Led2812b::NightSky() {
    if(_delayTrue && millis()-_waitStartMillis < _spd * THEME_NIGHTSKY_MULTIPLIER) {
        return;
    }

    //count- 15% of total leds
    if(_showBlack){
        setPixel(random(_noOfLeds), _clr);
        _brtTemp++;
        if(_brtTemp > _noOfLeds * 0.2)
            _showBlack = false;
    } else {
        setPixel(random(_noOfLeds), CRGB::Black);
        _brtTemp--;
        if(_brtTemp <= 0)
            _showBlack = true;
    }
    
    _updateLeds = true;
    _delayTrue = true;
    _waitStartMillis = millis();
}

void Led2812b::RGBLoop() {
    if(_delayTrue && millis()-_waitStartMillis < _spd * THEME_RGBLOOP_MULTIPLIER) {
        return;
    }

    if(_showBlack && _brtTemp == 0){
        if(_hueInt >= 2)
            _hueInt = 0;
        else
            _hueInt++;
    }

    if(_showBlack){
        _brtTemp++;
        if(_brtTemp > 255){
            _showBlack = false;
            _brtTemp = 255;
        }
    } else {
        _brtTemp--;
        if(_brtTemp <= 0){
            _showBlack = true;
            _brtTemp = 0;
        }
    }

    CRGB tclr;
    switch (_hueInt) {
        case 0:
            tclr.r = _brtTemp;
            tclr.g = 0;
            tclr.b = 0;
            break;
        case 1: 
            tclr.r = 0;
            tclr.g = _brtTemp;
            tclr.b = 0;
            break;
        case 2: 
            tclr.r = 0;
            tclr.g = 0;
            tclr.b = _brtTemp;
            break;
    }
    applyColor(tclr);
    _updateLeds = true;
    _delayTrue = true;
    _waitStartMillis = millis();
}

void Led2812b::theaterChaseRainbow() {
    if(_delayTrue && millis()-_waitStartMillis < _spd * THEME_THEATRE_MULTIPLIER) {
        return;
    }

    if(_hueInt == 0){
        if(_brtTemp < 256)
            _brtTemp++;
        else
            _brtTemp = 0;
    }

    if(_hueInt < 3)
        _hueInt++;
    else
        _hueInt = 0;

    for (int i = 1; i < _noOfLeds; i = i + 3) {
        setPixel(i + _hueInt-1, 0, 0, 0);    //turn every third pixel off
    }

    for (int i = 0; i < _noOfLeds; i = i + 3) {
        Wheel( (i + _brtTemp) % 255);
        setPixel(i + _hueInt, _cWheel[0], _cWheel[1], _cWheel[2]); //turn every third pixel on
    }

    _updateLeds = true;
    _delayTrue = true;
    _waitStartMillis = millis();
}

void Led2812b::Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    _cWheel[0] = WheelPos * 3;
    _cWheel[1] = 255 - WheelPos * 3;
    _cWheel[2] = 0;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    _cWheel[0] = 255 - WheelPos * 3;
    _cWheel[1] = 0;
    _cWheel[2] = WheelPos * 3;
  } else {
    WheelPos -= 170;
    _cWheel[0] = 0;
    _cWheel[1] = WheelPos * 3;
    _cWheel[2] = 255 - WheelPos * 3;
  }
}

void led2812bConfigureMqtt(const char * payload){
    DeserializationError err = deserializeJson(jsonDoc, payload);
    if(err){
        DEBUG_MSG_P(PSTR("[LED2812BCONF] JSON decode failed (%s)\n"), err.c_str());
        return;
    }

    JsonObject data = jsonDoc[JSON_LEDCONF];
    byte mode = data[CONF_MODE];
    byte noOfStrips = getSetting(K_NO_OF_LEDSTRIPS, 0).toInt();
    byte stripNumber;

    switch (mode) {
    case CONF_MODE_NEW:
        {
            byte gpio = data[CONF_GPIO].as<int>();// | 0xFF;
            byte ledsCount = data[CONF_NO_OF_LEDS];   // | RELAY_TYPE_INVERSE;
            setSetting(K_LEDSTRIP_PIN, noOfStrips, gpio);
            setSetting(K_LEDSTRIP_LEDCOUNT, noOfStrips, ledsCount);
            setSetting(K_NO_OF_LEDSTRIPS, noOfStrips+1);  //Increase the total count of relays
            DEBUG_MSG_P(PSTR("[LED2812BCONF] Adding new 2812b strip. GPIO %d, leds %d. Total now %d"),
                        gpio, ledsCount, noOfStrips+1);
        }
        break;
    
    case CONF_MODE_DELETE_ALL:
        setSetting(K_NO_OF_LEDSTRIPS, 0); 
        DEBUG_MSG_P(PSTR("[LED2812BCONF] Deleting all the 2812b strips."));
        break;

    case CONF_MODE_GET_SINGLE:
        stripNumber = data[CONF_LED_STRIP_NUMBER];

        if(noOfStrips <= stripNumber){
            DEBUG_MSG_P(PSTR("[LED2812BCONF] Invalid led strip number.\n"));
        } else {
            //jsonDoc.clear();
            JsonObject obj = jsonDoc.to<JsonObject>();
            JsonObject rconf = obj.createNestedObject(JSON_RCONF);
            rconf[CONF_MODE] = CONF_MODE_GET_SINGLE;
            rconf[CONF_LED_STRIP_NUMBER] = stripNumber;
            rconf[JSON_CONF_RESPONCE] = true;
            rconf[CONF_GPIO] = getSetting(K_LEDSTRIP_PIN, stripNumber, 0xFF);
            rconf[CONF_NO_OF_LEDS] = getSetting(K_NO_OF_LEDSTRIPS, stripNumber, 0xFF);

            mqttSend(MQTT_TOPIC_CONF, jsonDoc.as<String>().c_str());
        }
        break;

    case CONF_MODE_UPDATE_SINGLE:
        stripNumber = data[CONF_LED_STRIP_NUMBER];

        if(noOfStrips <= stripNumber){
            DEBUG_MSG_P(PSTR("[LED2812BCONF] Invalid button number.\n"));
        } else {
            byte gpio = data[CONF_GPIO].as<int>() | 0xFF;
            byte ledsCount = data[CONF_NO_OF_LEDS].as<int>() | 0xFF;
            setSetting(K_LEDSTRIP_PIN, stripNumber, gpio);
            setSetting(K_LEDSTRIP_LEDCOUNT, stripNumber, ledsCount);
            DEBUG_MSG_P(PSTR("[LED2812BCONF] Updating strip %d. GPIO %d, leds %d."),
                        stripNumber, gpio, ledsCount);
        }
        break;

    default:
        break;
    }

    jsonDoc.clear();
}

void updateLed2812bStrip(byte stripNo, byte theme, int16_t brt, int16_t spd, long clr){
    if (stripNo >= _strips.size()) {
        DEBUG_MSG_P(PSTR("[2812B] Wrong strip (%d)\n"), stripNo);
        return;
    }

    Led2812b* dstrip = _strips[stripNo];
    if(theme > 0){
        dstrip -> _theme = theme;
        dstrip->_repeat = true;
        dstrip->_clr = getSetting(K_LEDSTRIP_CLR, stripNo, DEFAULT_THEME_CLR).toInt();
        dstrip->_initVars();
        setSetting(K_LEDSTRIP_THM, stripNo, theme);
        DEBUG_MSG_P(PSTR("[2812B] Strip %d theme set to %d\n"), stripNo, theme);
    }
    if(brt > 0){
        dstrip -> _brt = brt;
        dstrip->_applyBrightness = true;
        setSetting(K_LEDSTRIP_BRIGHTNESS_ALL, brt);
        DEBUG_MSG_P(PSTR("[2812B] Strip %d brightness set to %d\n"), stripNo, brt);
    }
    if(spd > 0){
        dstrip -> _spd = spd;
        dstrip->_repeat = true;
        setSetting(K_LEDSTRIP_SPD, stripNo, spd);
        DEBUG_MSG_P(PSTR("[2812B] Strip %d speed set to %d\n"), stripNo, spd);
    }
    if(clr >= 0){
        dstrip->_clr = clr;
        dstrip->_repeat = true;
        if(dstrip->_theme == THEME_NIGHTSKY){
            dstrip->_applyClr = false;
        } else {
            dstrip->_applyClr = true; 
        }
        setSetting(K_LEDSTRIP_CLR, stripNo, clr);
        DEBUG_MSG_P(PSTR("[2812B] Strip %d color set to %d\n"), stripNo, clr);
    }

    

    publishStripStatus(stripNo);
}

void publishStripStatus(byte stripNo){
    if(stripNo > _strips.size())
        return;
    Led2812b* dstrip = _strips[stripNo];

    //Publish theme, brightness, speed, color
    mqttSend(MQTT_TOPIC_LEDSTRIP_THM, stripNo, String(dstrip->_theme).c_str());
    mqttSend(MQTT_TOPIC_LEDSTRIP_BRT, stripNo, String(dstrip->_brt).c_str());
    mqttSend(MQTT_TOPIC_LEDSTRIP_SPEED, stripNo, String(100 - dstrip->_spd).c_str());
    mqttSend(MQTT_TOPIC_LEDSTRIP_CLR, stripNo, String(dstrip->_clr).c_str());
}

void publishStripStatus(){
    for(byte i = 0; i < _strips.size(); i++){
        publishStripStatus(i);
    }
}

void led2812bMQTTCallback(unsigned int type, const char * topic, const char * payload) {
    if (type == MQTT_CONNECT_EVENT) {
        // Send status on connect
        publishStripStatus();
        //Subscribing to general topics in mqtt.cpp
        /*if(_strips.size() > 0){
            // Subscribe to own /set topic
            char relay_topic[strlen(MQTT_TOPIC_LEDSTRIP_THM) + 3];
            snprintf_P(relay_topic, sizeof(relay_topic), PSTR("%s/+"), MQTT_TOPIC_LEDSTRIP_THM);
            mqttSubscribe(relay_topic);
            snprintf_P(relay_topic, sizeof(relay_topic), PSTR("%s/+"), MQTT_TOPIC_LEDSTRIP_BRT);
            mqttSubscribe(relay_topic);
            snprintf_P(relay_topic, sizeof(relay_topic), PSTR("%s/+"), MQTT_TOPIC_LEDSTRIP_SPEED);
            mqttSubscribe(relay_topic);
            snprintf_P(relay_topic, sizeof(relay_topic), PSTR("%s/+"), MQTT_TOPIC_LEDSTRIP_CLR);
            mqttSubscribe(relay_topic);
        }  */ 
    }

    if (type == MQTT_MESSAGE_EVENT) {
        String t = mqttMagnitude((char *) topic);

        if(t.startsWith(MQTT_TOPIC_CONF)){
            led2812bConfigureMqtt(payload);

        } else if (t.startsWith(MQTT_TOPIC_LEDSTRIP_THM)) {
            byte id = t.substring(strlen(MQTT_TOPIC_LEDSTRIP_THM)+1).toInt();
            byte val = atoi(payload);
            updateLed2812bStrip(id, val, -1, -1, -1);

        } else if (t.startsWith(MQTT_TOPIC_LEDSTRIP_BRT)) {
            byte id = t.substring(strlen(MQTT_TOPIC_LEDSTRIP_BRT)+1).toInt();
            byte val = atoi(payload);
            updateLed2812bStrip(id, 0, val, -1, -1);

        } else if (t.startsWith(MQTT_TOPIC_LEDSTRIP_SPEED)) {
            byte id = t.substring(strlen(MQTT_TOPIC_LEDSTRIP_SPEED)+1).toInt();
            int val = atoi(payload);
            val = 100 - val;
            updateLed2812bStrip(id, 0, -1, val, -1);

        } else if (t.startsWith(MQTT_TOPIC_LEDSTRIP_CLR)) {
            byte id = t.substring(strlen(MQTT_TOPIC_LEDSTRIP_CLR)+1).toInt();
            long val = atoi(payload);
            updateLed2812bStrip(id, 0, -1, -1, val);

        }
    }

    if (type == MQTT_DISCONNECT_EVENT) {
        
    }
}

void led2812bSetup(){
    byte noOfStrips = getSetting(K_NO_OF_LEDSTRIPS, 0).toInt();
    for(byte i = 0; i < noOfStrips; i++){
        Led2812b* dstrip = new Led2812b(getSetting(K_LEDSTRIP_LEDCOUNT, i, 0).toInt(),
                                        getSetting(K_LEDSTRIP_PIN, i, GPIO_NONE).toInt());
        dstrip->_theme = getSetting(K_LEDSTRIP_THM, i, DEFAULT_THEME).toInt();
        dstrip->_brt = getSetting(K_LEDSTRIP_BRIGHTNESS_ALL, DEFAULT_THEME_BRT).toInt();
        dstrip->_spd = getSetting(K_LEDSTRIP_SPD, i, DEFAULT_THEME_SPD).toInt();
        dstrip->_clr = getSetting(K_LEDSTRIP_CLR, i, DEFAULT_THEME_CLR).toInt();

        dstrip->_repeat = true;
        dstrip->_applyBrightness = true;

        _strips.push_back(dstrip);
        DEBUG_MSG_P(PSTR("[2812B] Led strip gpio %d leds %d.\n"),
        getSetting(K_LEDSTRIP_PIN, i, GPIO_NONE).toInt(),
        getSetting(K_LEDSTRIP_LEDCOUNT, i, 0).toInt());
    }
    DEBUG_MSG_P(PSTR("[2812B] %d Led strip added.\n"), _strips.size());

    // Register loop
    espurnaRegisterLoop(led2812bLoop);
    mqttRegister(led2812bMQTTCallback);
}

void led2812bLoop(){
    for (unsigned int i=0; i < _strips.size(); i++) {
        _strips[i]->loop();
    }
}