/*************************MinifigLightningSystem Project by Jojo1220**********************************
  Filename:     MiniFigLightning.h
  Purpose:      MinifigLightning.h File for the MinifigLightning System Project
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

#ifndef MINIFIGLIGHTNING_H
#define MINIFIGLIGHTNING_H

#include "CustomDebugging.h"
#include "Preferences.h"

/*-----------------------------------------------------------------------------
 * Setup Global Constants
 * ----------------------------------------------------------------------------
*/
// WS2812b Pin used to controll LED's (Same Pin then Built-IN LED!)
#define PIN       2            //Communication PIN on ESP8266 -->> WS2812 ->> Watch out: Same as LEDBuiltin!!!

// Define Project Specific Default Vaules
#define DefaultSingelColor false
  #define DefaultRed 100
  #define DefaultGreen 100
  #define DefaultBlue 100

#define DefaultSingleAnimationMode false
#define DefaultAnimationStep 0

#define DefaultBrigthness 80

#define DefaultSpeed 5

#define DefaultNUMPIXELS 5      //Amount of WS2812 connected in series

// WiFi State Constants
#define ConnectWiFiSTAMode 99
#define StationModeStarted 100
#define StartWiFiAPMode 199
#define AccessPointModeStarted 200

// Define Static Variables for Wifi Access in Station Mode
// Edit them directly here for correct Init
#define STASSID "THIS IS YOUR WiFI SSID"
#define STAPSK  "THIS IS YOUR WIFI PASSWORD" // Be sure it is longer then 8 chars

// Define Static Variables for Wifi Access in AP Mode
#define APSSID "MiniFigLightning"
#define APPSK  "AdminFigure_23"

//Using Google DNS AP IP Address because of Captive-Portal Request on Android do not change!
#define AP_IP_ADDRESS 8,8,8,8
#define SUBNET_MASK 255,255,255,0

// Setting up second DNS Server IP for STA Mode
#define DNS_IP_ADDRESS 8,8,4,4

// Max LED's within Project Constant
#define MAX_ARRAY_SIZE 254 // Do NOT Use 255!

// Definition of OTA UPD Port
#define OTA_Port 8266

/*-----------------------------------------------------------------------------
 * Setup Custom Structs (Datatyps)
 * ----------------------------------------------------------------------------
*/
// Custom Struct for Project Specific Parameters
struct paramValues{
  char ssid[32] = "";
  char password[32] = "";
  bool singleColorMode = false;
  bool singleAnimationMode = false;
  uint8_t speed = 0;
  uint8_t AnimationStep = 0;
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t brightness = 0;
  uint8_t numLEDlenght = DefaultNUMPIXELS;
  uint8_t LedwithinEket[MAX_ARRAY_SIZE];
};

/*-----------------------------------------------------------------------------
 * Setup Global Variables
 * ----------------------------------------------------------------------------
*/

// Project specific System Parameters
extern paramValues systemParameters;

// Preferences Object for Remanent adjustable Settings Variables
extern Preferences preferences;

// MDNS Hostnma Projcect Specific
extern const char *myHostname; // MiniFigLightningSystem

// Preferences APP Name
extern const char *AppName;

// Definition of Project Specific Constant Key Char* for Preferences Access
extern const char* SSID_KEY;
extern const char* PASSWORD_KEY;
extern const char* SINGLECOLORMODE_KEY;
extern const char* SINGLEANIMATIONMODE_KEY;
extern const char* SINGLEANIMATIONSTEP_KEY;
extern const char* RED_KEY;
extern const char* GREEN_KEY;
extern const char* BLUE_KEY;
extern const char* BRIGHTNESS_KEY;
extern const char* LEDLENGTH_KEY;
extern const char* LEDASSIGNMENT_KEY;
extern const char* SPEED_KEY;

/*-----------------------------------------------------------------------------
 * Setup Custom Function Headers
 * ----------------------------------------------------------------------------
*/
void printParameters(paramValues _data);
void initializeArray(uint8_t size);

void preferencesBegin(void);

paramValues readParameters(void);
paramValues writeParameters(paramValues _data);

#endif // MiniFigLightning.h
