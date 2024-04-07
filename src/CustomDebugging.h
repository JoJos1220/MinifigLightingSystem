/*************************MinifigLightningSystem Project by Jojo1220**********************************
  Filename:     CustomDebugging.h
  Purpose:      CutomDebugging.h File for Debugging of MinifigLightning System Project
  Autor:        Jojo1220
  Created:      21.09.2023
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

#ifndef CUSTOMDEBUGGING_H
#define CUSTOMDEBUGGING_H

#include <Arduino.h>

/*-----------------------------------------------------------------------------
 * Debugging Serial Declaration
 * ----------------------------------------------------------------------------
*/

// Defining DEBUG Output Level for Console
#ifdef SERIAL_OUTPUT_DEBUGGING
  #define DEBUG_SERIAL Serial
#else
  #define DEBUG_SERIAL if(0) Serial
#endif

// Defining INFORMATION Output Level for Console
#ifdef SERIAL_OUTPUT_INFORMATION
  #define DEBUG_SERIAL_INFORMATION Serial
#else
  #define DEBUG_SERIAL_INFORMATION if(0) Serial
#endif

// Several More Debug Levels are Possible e.g.
// #define DEBUG_SERIAL_Information if(SERIAL_OUTPUT_DEBUGGING)Serial

/*-----------------------------------------------------------------------------
 * Function Header Declaration
 * ----------------------------------------------------------------------------
*/
void ESPDebuggingFunction(void);

#endif // CustomDebugging.h