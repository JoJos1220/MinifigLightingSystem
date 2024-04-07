/*************************MinifigLightningSystem Project by Jojo1220**********************************
  Filename:     Animation.cpp
  Purpose:      See Animation.h
  Autor:        Jojo1220
  Created:      24.04.2023
  Modified:     21.09.2023
  Lizenz:       CC BY-NC-ND 4.0
  Notification: See Animation.h
******************************************v1.0.0****************************************************/

#include "Animation.h"

/*-----------------------------------------------------------------------------
 * Animation Variables Declaration
 * ----------------------------------------------------------------------------
*/
uint16_t _i = 0;
unsigned long time_now = 0;
long firstPixelHue = 0;
uint16_t _firstPixelHue = 0;
uint16_t _b = 0;
uint16_t _c = 0;

/*-----------------------------------------------------------------------------
 * Animation Function: Set Brightness of LED's
 * ----------------------------------------------------------------------------
*/
void setBrightness(uint8_t bright, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip){
  bright = bright*255/100; // Scaling 0-100% Input to 0-255 UInt8_t Value from SetLuminance() Function
  _strip->SetLuminance(bright);
}

/*-----------------------------------------------------------------------------
 * Update Lengght: Setup new LED Length to Neopixels in Field
 * ----------------------------------------------------------------------------
*/
bool updateLength(uint16_t newLength, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip){
  if (_strip != NULL){
    delete _strip;
  }
  _strip = new NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>(newLength, PIN);
  if(_strip == NULL){
    DEBUG_SERIAL.println("OUT of Memory!");
    return false;
  }
  _strip->Begin();
  return true;
}

/*-----------------------------------------------------------------------------
 * Animation Function: Single Color for ALL LED's based on e.g. Webserver RGB Selector
 * ----------------------------------------------------------------------------
*/
bool singleColor(RgbColor _color, int _wait, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip){
 if(_i<_strip->PixelCount()){
    _strip->SetPixelColor(_i, _color); //  Set pixel's color (in RAM)
    _strip->Show();   //  Update strip to match

    _i++;
    return true;
  }else{
    if(asyncBreak(&time_now, _wait)){
      return true;
    };
    resetAnimationVariables();
    return false;
  }    
}

/*-----------------------------------------------------------------------------
 * Animation Function: colorWipe Animation
 * Fill strip pixels one after another with a color. Strip is NOT cleared
 * first; anything there will be covered pixel by pixel. Pass in color
 * (as a single 'packed' 32-bit value, which you can get by calling
 * strip.Color(red, green, blue) as shown in the loop() function above),
 * and a delay time (in milliseconds) between pixels.
 * ----------------------------------------------------------------------------
*/
bool colorWipe(RgbColor _color, int _wait, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip) {
  if(_i<_strip->PixelCount()){
    ledEketAssignment(_i,_color, _strip);
    _strip->Show();                        //  Update strip to match
    if(asyncBreak(&time_now, _wait)){
      return true;
    };                         //  Pause for a moment
    _i++;
    return true;
  }else{
    clearALLPixels(_strip);
    resetAnimationVariables();
    return false;
  }
}

/*-----------------------------------------------------------------------------
 * Animation Function: theaterChase Animation
 * Theater-marquee-style chasing lights. Pass in a color (32-bit value,
 * a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
 * between frames.
 * ----------------------------------------------------------------------------
*/
bool theaterChase(RgbColor _color, int _wait, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip){
  if (_i < 10) {// Repeat 10 times...
    _strip->ClearTo(RgbColor(0,0,0));
    for (_c = _b; _c < _strip->PixelCount(); _c += 3){
      ledEketAssignment(_c,_color, _strip);
    }
    _strip->Show();
    if(asyncBreak(&time_now, _wait)){
      return true;
    };
    _b++;
     
    if (_b >= 3) {
      _b = 0;
      _i++;
    }
    return true;
  } else {
    clearALLPixels(_strip);
    resetAnimationVariables();
    return false;
  }  
}

/*-----------------------------------------------------------------------------
 * Animation Function: rainbow Animation
 * Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
 * Hue of first pixel runs 5 complete loops through the color wheel.
 * Color wheel has a range of 65536 but it's OK if we roll over, so
 * just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
 * means we'll make 5*65536/256 = 1280 passes through this loop:
 * Calculate the color wheel step for each pixel
 * ----------------------------------------------------------------------------
*/
bool rainbow(int _wait, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip){
  uint16_t colorWheelStep = 360 / _strip->PixelCount();

  // Iterate through all pixels using a conditional statement
  if (firstPixelHue < 5 * 360)
  {
    for (; _i < _strip->PixelCount(); _i++){
      // Calculate the hue offset for the current pixel
      uint16_t hue = firstPixelHue + (_i * colorWheelStep);

      // Convert hue to RGB color
      RgbColor color = WheelColor(hue);

      // Set pixel color
      ledEketAssignment(_i, color, _strip);

      // Move to the next pixel
    };
    _i = 0;
    // Update strip with new contents
    _strip->Show();

    // Pause for a moment
    if(asyncBreak(&time_now, _wait)){
      return true;
    };
    firstPixelHue += 4;
    return true;
  }
  else
  {
    clearALLPixels(_strip);
    // If we have processed all pixels, reset the counter and _firstPixelHue
    resetAnimationVariables();
    return false;
  }
}

/*-----------------------------------------------------------------------------
 * Animation Function: theaterChaseRainbow Animation
 * Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
 * ----------------------------------------------------------------------------
*/
bool theaterChaseRainbow(int _wait, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip){
 if (_i < 30){ // Repeat 30 times...
    if (_b < 3){                 //  'b' counts from 0 to 2...
    _strip->ClearTo(RgbColor(0,0,0)); //   Set all pixels in RAM to 0 (off)
    // 'c' counts up from 'b' to end of strip in increments of 3...
    for (_c = _b; _c < _strip->PixelCount(); _c += 3){
      // hue of pixel 'c' is offset by an amount to make one full
      // revolution of the color wheel (range 65536) along the length
      // of the strip (strip.numPixels() steps):

      uint16_t hue = _firstPixelHue + _c * 360 / _strip->PixelCount();

      RgbColor color = WheelColor(hue);

      ledEketAssignment(_c, color, _strip);
    }
    _strip->Show();              // Update strip with new contents
    if(asyncBreak(&time_now, _wait)){
        return true;
    };                 // Pause for a moment
    _firstPixelHue += 12;
    _b++;
    }else{
      _b = 0;
      _i++;
    }
    return true;
  }
  else{
    clearALLPixels(_strip);
    resetAnimationVariables();
    return false;
  }
}

/*-----------------------------------------------------------------------------
 * Animation Function: clearAllPixels - set all Pixels OFF during e.g. startup
 * ----------------------------------------------------------------------------
*/
bool clearALLPixels(NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip){
  _strip->ClearTo(RgbColor(0,0,0));
  return false;
}

/*-----------------------------------------------------------------------------
 * Animation Function: ledEKETAssingment - Function to assign e.g. 2 LED's to one EKET
 *  by the given LedwithinEket array
 * ISSUE may be that LED's can be lighting up twice in series
 * ----------------------------------------------------------------------------
*/
void ledEketAssignment(uint16_t var, RgbColor color, NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* _strip){
  for(uint16_t __i = 0; __i<_strip->PixelCount();__i++){
    if(var == systemParameters.LedwithinEket[__i]){
      _strip->SetPixelColor(__i, color); //  Set pixel's color (in RAM)
    }
  }
} 

/*-----------------------------------------------------------------------------
 * AsyncBreak Function
 * ----------------------------------------------------------------------------
*/
bool asyncBreak(unsigned long* _timeNow, int _period){
  if(millis() >= (*_timeNow + _period)){
    *_timeNow += _period;
    return false;
  }else{
    return true;
  }
}

/*-----------------------------------------------------------------------------
 * Reset Animation Variables after every loop
 * ----------------------------------------------------------------------------
*/
void resetAnimationVariables(void){
  _i = 0;
  firstPixelHue = 0;
  _firstPixelHue = 0;
  _b = 0;
  _c = 0;
}

/*-----------------------------------------------------------------------------
 * Animation Function: WheelColor - RGB Animation Wheel for R/G/B Color based on HslColor() - Function
 * ----------------------------------------------------------------------------
*/
RgbColor WheelColor(uint16_t wheelValue) {
  wheelValue = wheelValue % 360;
  // divide the wheelValue by 360.0f to get a value between 0.0 and 1.0 needed for HslColor
  return HslColor(wheelValue / 360.0f, 1.0f, 0.5f); // this will autoconvert back to RgbColor
}
