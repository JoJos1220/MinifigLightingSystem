/*************************MinifigLightningSystem Project by Jojo1220**********************************
  Filename:     Animation.h
  Purpose:      Animation.h File for the MinifigLightning System Project
  Autor:        Jojo1220
  Created:      24.04.2023
  Modified:     21.09.2023
  Lizenz:       CC BY-NC-ND 4.0
  Notification: Requieres the following Libarys:
                  ESP Async WebServer @ ^1.2.3
	                Preferences @ ^2.1.0
	                NeoPixelBus @ ^2.7.9
                  GDBStup @ ^0.3  (optional if acitavted)
                  LittleFS @ 0.1.0
                  DNSServer @ 1.1.1
                  ESP8266mDNS @ 1.2
                  ESP8266WiFi @ 1.0
******************************************v1.0.0****************************************************/

#ifndef ANIMATION_H
#define ANIMATION_H

#include "MiniFigLightning.h"

#include "NeoPixelBusLg.h"


/*-----------------------------------------------------------------------------
 * Typdef Declaration
 * ----------------------------------------------------------------------------
*/

// NeoPixelBusLg Constant used LED Definition
typedef NeoRgbFeature ConstNeoGrbFeature;
typedef NeoEsp8266Uart1800KbpsMethod ConstNeoEsp8266Uart1Ws2812xMethod;

/*-----------------------------------------------------------------------------
 * Function Header Declaration
 * ----------------------------------------------------------------------------
*/

bool asyncBreak(unsigned long* _timeNow, int _period);
void setBrightness(uint8_t bright, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip);
bool updateLength(uint16_t newLength, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip);
void ledEketAssignment(uint16_t var, RgbColor color, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip);
bool clearALLPixels(NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip);

void resetAnimationVariables(void);

// "true" Async-Animation Functions
bool singleColor(RgbColor _color, int _wait, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip);
bool colorWipe(RgbColor _color, int _wait, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip);
bool theaterChase(RgbColor _color, int _wait, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip);
bool rainbow(int _wait, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip);
bool theaterChaseRainbow(int _wait, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip);

//Wheel Color: Converts 0-360° Uint Value into RGBColor
RgbColor WheelColor(uint16_t wheelValue);

#endif // Animation.h
