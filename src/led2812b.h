#ifndef LED2812B_H
#define LED2812B_H

#include <FastLED.h>


#include "relay.h"
#include "mqtt.h"
#include "settings.h"
#include "debug.h"

#define STRIP_TYPE WS2812B
#define COLOR_MODE GRB
#define DELAY_APPLY_COLOR_ALL 2

#define THEME_OFF         1 
#define THEME_ON          2 //Not implemented
#define THEME_RAINBOW     3 
#define THEME_STROBE      4 
#define THEME_ALLWHITE    5 
#define THEME_COLORFUL    6 
#define THEME_FADE        7 
#define THEME_NIGHTSKY    8
#define Theme_RGBLoop                 9
#define Theme_theaterChaseRainbow     10
#define Theme_HueEffect               11

#define DEFAULT_THEME Theme_HueEffect
#define DEFAULT_THEME_BRT 100
#define DEFAULT_THEME_SPD 10
#define DEFAULT_THEME_CLR 0x00FF00

#define THEME_FADE_MULTIPLIER 0.5
#define THEME_NIGHTSKY_MULTIPLIER 20
#define THEME_RGBLOOP_MULTIPLIER 0.05
#define THEME_THEATRE_MULTIPLIER 0.5
#define THEME_HUEEFFECT_MULTIPLIER 0.5

class Led2812b {
    public:
        byte _theme;
        byte _brt; //Brightness
        int16_t _brtTemp = 0; //Temp Brightness/general counter
        byte _spd; //effect speed
        bool _repeat; //Repet the selected effect
        bool _applyBrightness; //If true, change brightness.
        bool _applyClr;
        bool _showBlack; //Used for various purpose including switch between loops/ifs etc
        bool _delayTrue;
        bool _updateLeds; //if true FastLED.show();
        CRGB* _leds;
        uint8_t* _hue;
        int _hueInt; //Used in rainbow and for some counter
        CRGB _clr; //Color to apply to all the leds
        uint16_t _noOfLeds;
        unsigned long _waitStartMillis;
        byte _cWheel[3];

        Led2812b(uint16_t noOfLeds, byte ledGpio);
        ~Led2812b();

        void loop();
        void _initVars();

    private:
        void _setupFastLed(uint16_t noOfLeds, uint8_t gpio);
        void setPixel(int Pixel, byte red, byte green, byte blue);
        void setPixel(int Pixel, CRGB clr);
        void setAll(byte red, byte green, byte blue);
        //void setAll(long color);
        void applyColor(CRGB clr);
        void HueEffect();
        void simpleFade();
        void NightSky();
        void RGBLoop();
        void theaterChaseRainbow();
        void Wheel(byte WheelPos);
        //void RGBLoop(uint16_t SpeedDelay);


};

void led2812bSetup();
void led2812bLoop();
void publishStripStatus(byte stripNo);
void publishStripStatus();

#endif