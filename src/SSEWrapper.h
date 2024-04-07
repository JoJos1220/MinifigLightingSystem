/*************************MinifigLightningSystem Project by Jojo1220**********************************
  Filename:     SSEWapper.h
  Purpose:      SSEWrapper.h File Wrappeing SSE Functionalities to the MiniFigLightningSystem Project
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

#ifndef SSEWRAPPER_H
#define SSEWRAPPER_H

#include "Arduino.h"

#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include "flash_hal.h"
#include "ESPAsyncWebServer.h"
#include <CustomDebugging.h>

/*-----------------------------------------------------------------------------
 * Setup Global Constants
 * ----------------------------------------------------------------------------
*/
// Defining MAX. Shown Network WiFi's
#define MAXSHOWNWIFINETWORKS 5

// Struct for OTA
struct paramOTA{
    int command = 0;
    int localPort = 0;
    int contentSize = 0;
    char md5CheckSum[33] = "";  
    uint16_t port = 0;
    IPAddress IP;
    char password[33] = "";
    char nonce[33] = "";
};

typedef enum {
  OTA_IDLE,
  OTA_WAITAUTH,
  OTA_RUNUPDATE
} ota_state_t;

/*-----------------------------------------------------------------------------
 * Setup custom SSEWrapper() -- Class
 * ----------------------------------------------------------------------------
*/
class SSEWrapper {
public:
  SSEWrapper(AsyncWebServer *globServer, AsyncEventSource *globEvents, const char* hostname);
  ~SSEWrapper();
  void setup(void (*SSEhandleRequest)(AsyncWebServerRequest *request));
  bool getRestart(void);
  void setRestart(bool _restart);

private:
  // "Main" Variables for ESPAsyncWebserver.h Libary used
  AsyncWebServer *_server;
  AsyncEventSource *_events;

  // Function Headers
  static boolean captivePortal(AsyncWebServerRequest *request);
  static boolean isIp(const char* str);

  static void notFound(AsyncWebServerRequest *request);
  static void WifiScanhandleRequest(AsyncWebServerRequest *request);
  static void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);

  // private static Variables for Update
  static size_t content_len;
  static size_t old_size;
  static bool restart;

  // private static Variable to store 5000ms delay for ScanWifi();
  static unsigned long RequestScanDelay;

  // Hostname for Redirecting captive Portal
  static const char* _myHostname;

  // char Puffer for Operation to avoid String
  static char CaptivePortalBuffer[24];
  static char SSEWrapperPuffer[55];
  static char responseBuffer[512];

};

#endif // SSEWrapper.h
