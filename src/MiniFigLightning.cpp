/*************************MinifigLightningSystem Project by Jojo1220**********************************
  Filename:     MiniFigLightning.cpp
  Purpose:      See MiniFigLightning.h
  Autor:        Jojo1220
  Created:      24.04.2023
  Modified:     21.09.2023
  Lizenz:       CC BY-NC-ND 4.0
  Notification: See MiniFigLightning.h
******************************************v1.0.0****************************************************/

#include "MiniFigLightning.h"

/*-----------------------------------------------------------------------------
 * Setup Global Variables
 * ----------------------------------------------------------------------------
*/

// Procject Specific System Parameters Instance
paramValues systemParameters;

// preferences instance
Preferences preferences;

// hostname for mDNS Service
const char *myHostname = "MiniFigLightningSystem";

// Preferences App Name
const char *AppName = "mAp";

// key constance or Access key:values within Preferences
const char* SSID_KEY = "ssid";
const char* PASSWORD_KEY = "password";
const char* SINGLECOLORMODE_KEY = "SingleMode";
const char* SINGLEANIMATIONMODE_KEY = "SingleAni";
const char* SINGLEANIMATIONSTEP_KEY = "AniStep";
const char* RED_KEY = "red";
const char* GREEN_KEY = "green";
const char* BLUE_KEY = "blue";
const char* BRIGHTNESS_KEY = "bright";
const char* LEDLENGTH_KEY = "length";
const char* LEDASSIGNMENT_KEY = "mAr_";
const char* SPEED_KEY = "speed";

// Char Buffer Variable
char InitArrayPuffer[10];

/*-----------------------------------------------------------------------------
 * Read Parameters from Preferences function
 * ----------------------------------------------------------------------------
*/
paramValues readParameters(void){
  paramValues _data;

  preferencesBegin();
  // Read Stored Wifi Settings in Preferences to _data struct
  snprintf(_data.ssid, sizeof(_data.ssid), "%s", preferences.getString(SSID_KEY).c_str());
  snprintf(_data.password, sizeof(_data.password), "%s", preferences.getString(PASSWORD_KEY).c_str());

  // Read Stored Project Specific Settings in Preferneces to _data struct
  _data.numLEDlenght = preferences.getUChar(LEDLENGTH_KEY);
  _data.singleColorMode = preferences.getBool(SINGLECOLORMODE_KEY);
  
  _data.r = preferences.getUChar(RED_KEY);
  _data.g = preferences.getUChar(GREEN_KEY);
  _data.b = preferences.getUChar(BLUE_KEY);

  _data.singleAnimationMode =  preferences.getBool(SINGLEANIMATIONMODE_KEY);
  _data.AnimationStep = preferences.getUChar(SINGLEANIMATIONSTEP_KEY);

  _data.brightness = preferences.getUChar(BRIGHTNESS_KEY);

  _data.speed = preferences.getUChar(SPEED_KEY);

  preferences.end();

  // Checking by default brightness Level "0" if so, factory settings has to be defined.
  if(_data.brightness == 0){
    DEBUG_SERIAL.println("[Prefs] not Found! Load Factory Settings!");
    _data = writeParameters(_data);
  }
  
  printParameters(_data);
  return _data;
}

/*-----------------------------------------------------------------------------
 * Write Parameters to Preferences function
 * ----------------------------------------------------------------------------
*/
paramValues writeParameters(paramValues _data){
  // Setting Default Variables to _data Struct
  _data.singleColorMode = DefaultSingelColor;
  _data.singleAnimationMode = DefaultSingleAnimationMode;
  _data.AnimationStep = DefaultAnimationStep;
  _data.r = DefaultRed;
  _data.g = DefaultGreen;
  _data.b = DefaultBlue;
  _data.brightness = DefaultBrigthness;
  _data.speed = DefaultSpeed;
  _data.numLEDlenght = DefaultNUMPIXELS;

  // Wrting Default Values Back to preferences
  preferencesBegin();

  // WiFi Settings applied seperatly!
  if (strlen(_data.ssid) <=1){
    snprintf(_data.ssid, sizeof(_data.ssid), "%s", STASSID);
    preferences.putString(SSID_KEY, _data.ssid);
    snprintf(_data.password, sizeof(_data.password), "%s", STAPSK);
    preferences.putString(PASSWORD_KEY, _data.password);
  }

  preferences.putBool(SINGLECOLORMODE_KEY, _data.singleColorMode);
  preferences.putBool(SINGLEANIMATIONMODE_KEY, _data.singleAnimationMode);
  preferences.putUChar(SINGLEANIMATIONSTEP_KEY, _data.AnimationStep);
  preferences.putUChar(RED_KEY, _data.r);
  preferences.putUChar(GREEN_KEY, _data.g);
  preferences.putUChar(BLUE_KEY, _data.b);
  preferences.putUChar(LEDLENGTH_KEY, _data.numLEDlenght);
  preferences.putUChar(BRIGHTNESS_KEY, _data.brightness);
  preferences.putUChar(SPEED_KEY, _data.speed);
    
  preferences.end();
  
  DEBUG_SERIAL.println("[Prefs] Factory Reset");

  return _data;
};

/*-----------------------------------------------------------------------------
 * Print Parameters function
 * ----------------------------------------------------------------------------
*/
void printParameters(paramValues _data){
  DEBUG_SERIAL.print("-------------------------------------------------");
  DEBUG_SERIAL.print("\nSettings:\tSingleColorMode: ");
  DEBUG_SERIAL.print(_data.singleColorMode ? "false" : "true");
  DEBUG_SERIAL.print("\tColor rgb: ");
  DEBUG_SERIAL.print(_data.r);
  DEBUG_SERIAL.print(",");
  DEBUG_SERIAL.print(_data.g);
  DEBUG_SERIAL.print(",");
  DEBUG_SERIAL.print(_data.b);
  DEBUG_SERIAL.print("\nSingleAnimationMode: ");
  DEBUG_SERIAL.print(_data.singleAnimationMode ? "false" : "true");
  DEBUG_SERIAL.print("\tAnimation Step: ");
  DEBUG_SERIAL.print(_data.AnimationStep);
  DEBUG_SERIAL.print("\tBrigthness: ");
  DEBUG_SERIAL.print(_data.brightness);
  DEBUG_SERIAL.print("\tSpeed: ");
  DEBUG_SERIAL.print(_data.speed);
  DEBUG_SERIAL.print("\tLED counts: ");
  DEBUG_SERIAL.print(_data.numLEDlenght);
  DEBUG_SERIAL.print("\nWiFi SSID: ");
  DEBUG_SERIAL.print(_data.ssid);
  DEBUG_SERIAL.print(" Password: ");
  DEBUG_SERIAL.print(_data.password);
  DEBUG_SERIAL.println("-------------------------------------------------");
}

/*-----------------------------------------------------------------------------
 * Initialization Function: Set LED Array Vector
 * ----------------------------------------------------------------------------
*/
void initializeArray(uint8_t _size) {
  // Reading Assignments from Preferences Back into Array
  DEBUG_SERIAL.println("LED Assign.: ");
  preferencesBegin();
  for(uint8_t i = 0; i<_size; i++){
    snprintf(InitArrayPuffer, sizeof(InitArrayPuffer), "%s%d", LEDASSIGNMENT_KEY, i);
   uint8_t ass = preferences.getUChar(InitArrayPuffer,255);
    // "255" is "not-Assignet-yet" Value -->> i it is set -->> default loop variable is used to assign
    if(ass != 255){
      systemParameters.LedwithinEket[i] = ass;
    }else{
      preferences.putUChar(InitArrayPuffer,i);
      systemParameters.LedwithinEket[i] = i;
    }
    
    DEBUG_SERIAL.print("LED Nr. :");
    DEBUG_SERIAL.print(i);
    DEBUG_SERIAL.print(" = ");
    DEBUG_SERIAL.println(systemParameters.LedwithinEket[i]);
    
    yield();

  }

  // Delete old Assignment Values from NVS to synchronize with Webserver and give
  // a "Default" starting point
  for(uint8_t i = _size; i<=254; i++){
    snprintf(InitArrayPuffer, sizeof(InitArrayPuffer), "%s%d", LEDASSIGNMENT_KEY, i);

    if(preferences.isKey(InitArrayPuffer)){
      preferences.remove(InitArrayPuffer);
    };

    yield();

  }

  preferences.end();
}

/*-----------------------------------------------------------------------------
 * Seperatly Preferences Begin Function
 * ----------------------------------------------------------------------------
*/
void preferencesBegin(void){
  uint8_t maxAttempts = 10;  // Max Attempts for Opening Preferences
  for(uint8_t attempt = 1; attempt <= maxAttempts; attempt++) {
    if(preferences.begin(AppName, false)) {
        // Successfull Initialization of Preferences
        break;  // Break-up-Loop
    }else{
      for(unsigned long start = millis(); millis() - start < 200;) {
        yield(); //short yield() back to ESP Firmware
      }       
      DEBUG_SERIAL.println("[Prefs] Begin!");
    }

    if (attempt == maxAttempts) {
      DEBUG_SERIAL.println("[Prefs] Setup Error Restart!");
      #ifndef NATIVE_ENVIRONMENT
        ESP.restart();
      #endif
    }
  }
}
