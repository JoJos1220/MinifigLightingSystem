/*************************MinifigLightningSystem Project by Jojo1220**********************************
  Filename:     SSEWapper.cpp
  Purpose:      See SSEWrapper.h
  Created:      24.04.2023
  Modified:     21.09.2023
  Lizenz:       CC BY-NC-ND 4.0
  Notification: See SSEWrapper.h
******************************************v1.0.0****************************************************/
#include <SSEWrapper.h>

size_t SSEWrapper::content_len = 0;
size_t SSEWrapper::old_size = 0;
bool SSEWrapper::restart = false;
unsigned long SSEWrapper::RequestScanDelay = millis();
const char* SSEWrapper::_myHostname = nullptr;
char SSEWrapper::responseBuffer[512];
char SSEWrapper::SSEWrapperPuffer[55];
char SSEWrapper::CaptivePortalBuffer[24];

/*-----------------------------------------------------------------------------
 * SSEWrapper::SSEWrapper() -- Konstructor Method
 * ----------------------------------------------------------------------------
*/
SSEWrapper::SSEWrapper(AsyncWebServer *globServer, AsyncEventSource *globEvents, const char* hostname) :
        _server(globServer), _events(globEvents) {_myHostname = hostname;};

/*-----------------------------------------------------------------------------
 * SSEWrapper::~SSEWrapper() -- Destructor Method
 * ----------------------------------------------------------------------------
*/
SSEWrapper::~SSEWrapper(){}

/*-----------------------------------------------------------------------------
 * SSEWrapper::setup() -- Setup Method
 * ----------------------------------------------------------------------------
*/
void SSEWrapper::setup(void (*SSEhandleRequest)(AsyncWebServerRequest *request)) {
  // All Webserver Handler Requests has to be specified here
  // -->> otherwise it is 404 not Found called!!
 
  // other Requests -->> Specific SSEhandleRequest in MAIN by if/else if
  _server->on("/", HTTP_GET, SSEhandleRequest);
  _server->on("/savePreferences", HTTP_POST, SSEhandleRequest);
  _server->on("/singleRGBcolor", HTTP_POST, SSEhandleRequest);
  _server->on("/brightness", HTTP_POST, SSEhandleRequest);
  _server->on("/singleColor", HTTP_POST, SSEhandleRequest);
  _server->on("/animationMode", HTTP_POST, SSEhandleRequest);
  _server->on("/updateLEDLength", HTTP_POST, SSEhandleRequest);
  _server->on("/updateSpecLED", HTTP_POST, SSEhandleRequest);
  _server->on("/updateSpeed", HTTP_POST, SSEhandleRequest);
  _server->on("/scanWifi", HTTP_GET, WifiScanhandleRequest);

  // Implementing of OTA functionalities
  _server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<head> OK - Update Called!</head><body>Please wait</body>");
  });
  _server->on("/doUpdate", HTTP_POST, [](AsyncWebServerRequest *request) {},
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
                  size_t len, bool final) {handleDoUpdate(request, filename, index, data, len, final);}
  );

  // Serving Static Requests based on Webserver Functionality:
  _server->on("/images/favicon.png", HTTP_GET, SSEhandleRequest);
  _server->on("/images/git.png", HTTP_GET, SSEhandleRequest);
  _server->on("/images/instruct.png", HTTP_GET, SSEhandleRequest);

  // Webserver Files (style and script)
  _server->on("/style.css", HTTP_GET, SSEhandleRequest);
  _server->on("/script.js", HTTP_GET, SSEhandleRequest);

  //Adding Event Handler to SSE Webserver in WiFi STA Mode
  _server->addHandler(_events).setFilter(ON_STA_FILTER);

  //Setup onNotFound Funktion for Webserver
  _server->onNotFound(SSEWrapper::notFound);
}

/*-----------------------------------------------------------------------------
 * SSEWrapper::notFound() -- notFound Method
 * ----------------------------------------------------------------------------
*/
void SSEWrapper::notFound(AsyncWebServerRequest *request){
  unsigned long deltaT = millis();
  if(SSEWrapper::captivePortal(request)) {  // If captive portal redirect instead of displaying the error page.
    //Do Nothing
    return;
  }else{
    DEBUG_SERIAL.println("Not Found!");
    request->send(404, "text/plain", "Not found");
    DEBUG_SERIAL_INFORMATION.print("notFound [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);
  }
}

/*-----------------------------------------------------------------------------
 * SSEWrapper::handleDoUpdate() -- UpdateHandler OTA Method
 * ----------------------------------------------------------------------------
*/
void SSEWrapper::handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  unsigned long deltaT = millis();
  if (!index){
    DEBUG_SERIAL.print("[OTA] Update: ");
    DEBUG_SERIAL.println(filename);
    SSEWrapper::content_len = request->contentLength();
    DEBUG_SERIAL.print("[OTA] Filesize recived [KB]: ");
    DEBUG_SERIAL.println(SSEWrapper::content_len);
    
    // if filename includes spiffs/littlefs, update the spiffs/littlefs partition
    int cmd = ((filename.indexOf("spiffs") > -1) || (filename.indexOf("littlefs") > -1)) ? U_FS : U_FLASH;
    Update.runAsync(true);
    
      uint32_t update_size = 0;
      // Filesystem (SPIFFS // LITTLEFS)
      if (cmd == U_FLASH){
        update_size = ((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
        DEBUG_SERIAL.print("Filesystem Update Size [kB]: ");
        DEBUG_SERIAL.println(update_size);
      } // Firmware
      else if (cmd == U_FS){
        update_size = ((size_t) &_FS_end - (size_t) &_FS_start);
        DEBUG_SERIAL.print("LittleFS Update Size [kB]: ");
        DEBUG_SERIAL.println(update_size);
      }
    
    //if (!Update.begin(SSEWrapper::content_len, cmd)) {
    if (!Update.begin(update_size, cmd)) {
      #ifdef SERIAL_OUTPUT_DEBUGGING
        Update.printError(Serial);
      #endif
    }

    request->send(200, "text/html", "<head><meta http-equiv='refresh' content='20;URL=/'/></head><body>Upload started! Please wait while the device reboots</body>");
  }

  if(len){
    if (Update.write(data, len) != len) {
      #ifdef SERIAL_OUTPUT_DEBUGGING
        Update.printError(Serial);
      #endif
    }else{
      size_t temp_size = (Update.progress()*100)/Update.size();
      if(old_size != temp_size){
        DEBUG_SERIAL.print("[OTA] Progress: ");
        DEBUG_SERIAL.println(temp_size);
        old_size = temp_size;
      }
    }
  }

  if(final){
    if(Update.hasError()){
      DEBUG_SERIAL.println(Update.getErrorString());
    }
    if(!Update.end(true)){
      #ifdef SERIAL_OUTPUT_DEBUGGING
        Update.printError(Serial);
      #endif
    } else {
      DEBUG_SERIAL.println("[OTA] Update complete!");
      DEBUG_SERIAL.println("[OTA] Reboot!");
      restart = true;
    }
  }
  DEBUG_SERIAL_INFORMATION.print("handleDoUpdate [ms]: ");
  DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);
}

/*-----------------------------------------------------------------------------
 * SSEWrapper::scanWifi() -- WiFiScanning Method
 * ----------------------------------------------------------------------------
*/
void SSEWrapper::WifiScanhandleRequest(AsyncWebServerRequest *request) {
  unsigned long deltaT = millis();
  DEBUG_SERIAL.println("[WiFi] Scan started...");

  ESPDebuggingFunction();

  if(millis() > RequestScanDelay + 5000UL){
    RequestScanDelay = millis();

    uint8_t n = WiFi.scanComplete();

    if(n == -2){
      WiFi.scanNetworks(true, true);
    }
    else if(n >= 0){
      // Limitating to MAX 5 Networks (reducing size & Communication load with client)
      if(n > MAXSHOWNWIFINETWORKS){
        n = MAXSHOWNWIFINETWORKS;
      }

      ESPDebuggingFunction();

      snprintf(responseBuffer, sizeof(responseBuffer), "[");

      for (uint8_t i = 0; i < n; ++i){
        if(i) strcat(responseBuffer, ",");
        snprintf(responseBuffer + strlen(responseBuffer), sizeof(responseBuffer) - strlen(responseBuffer),
          "{\"rssi\":%d,\"ssid\":\"%s\",\"bssid\":\"%s\",\"channel\":%d,\"secure\":%d,\"hidden\":%s}",
          WiFi.RSSI(i),
          WiFi.SSID(i).c_str(),
          WiFi.BSSIDstr(i).c_str(),
          WiFi.channel(i),
          WiFi.encryptionType(i),
          WiFi.isHidden(i) ? "true" : "false"
        );
      }
      WiFi.scanDelete();

      if(WiFi.scanComplete() == -2){
        WiFi.scanNetworks(true, true);
      }

      snprintf(responseBuffer + strlen(responseBuffer), sizeof(responseBuffer) - strlen(responseBuffer), "]");

      ESPDebuggingFunction();
      DEBUG_SERIAL.println(responseBuffer);

      request->send(200, "application/json", responseBuffer);

      ESPDebuggingFunction();
    }
  }else{
    DEBUG_SERIAL.print("Scan Not Ready yet! wait [ms]: ");
    DEBUG_SERIAL.println(5000UL - (millis() - RequestScanDelay));
    request->send(200, "text/plain", "not ready yet!");
  }

  DEBUG_SERIAL_INFORMATION.print("WifiScanhandleRequest [ms]: ");
  DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);
}

/*-----------------------------------------------------------------------------
 * SSEWrapper::captivePortal() -- Captive Portal Request
 * Redirect to captive portal if we got a request for another domain.
 * Return true in that case so the page handler do not try to handle the request again.
 * ----------------------------------------------------------------------------
*/
boolean SSEWrapper::captivePortal(AsyncWebServerRequest *request){
  unsigned long deltaT = millis();
  DEBUG_SERIAL.println("[SSE] captivePortal");
  // First: Checking if URL request is a allowed URL from "WhiteList"
  // Buffer "complete" Hostname with .local for Redirection URL
  // or Buffer with IP Address of AP
  
  //Reorganisation of softAP IP-Address
  IPAddress apAddress = WiFi.softAPIP();
  byte apIPAddress[4];
  apIPAddress[0] = apAddress[0];
  apIPAddress[1] = apAddress[1];
  apIPAddress[2] = apAddress[2];
  apIPAddress[3] = apAddress[3]; 

  snprintf(SSEWrapperPuffer, sizeof(SSEWrapperPuffer), "http://%s.local", _myHostname);
  snprintf(CaptivePortalBuffer, sizeof(CaptivePortalBuffer), "http://%d.%d.%d.%d/", apIPAddress[0],apIPAddress[1],apIPAddress[2],apIPAddress[3]);
  
  // Checking if Request is an IP-Address and Request is unequal "complete" Hostname//IP-Address (avoiding redirect Overflow to ESP8266!!)
  // The captivePortal routine checks if the incoming request is targeted to our local controllers web server IP.
  // In case it’s not, the address is redirected to the local controller IP.
  if (!isIp(request->host().c_str()) && strcmp(request->host().c_str(), CaptivePortalBuffer)!= 0 && strcmp(request->host().c_str(), SSEWrapperPuffer) != 0) {

    DEBUG_SERIAL.print("[CP] Request redirected: ");
    DEBUG_SERIAL.println(SSEWrapperPuffer);
    ESPDebuggingFunction();

    // Redirecting Client Request and STOP connection (Avoidng Overflow to ESP8266!!)
    request->redirect(SSEWrapperPuffer);
    request->client()->stop();

    DEBUG_SERIAL_INFORMATION.print("captivePortal <OK> [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);
    return true;
  }else{
    DEBUG_SERIAL_INFORMATION.print("captivePortal <NOK>[ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);
    return false;
  }
}

/*-----------------------------------------------------------------------------
 * SSEWrapper::isIp() -- Checking if Request is a IP. Is this an IP?
 * ----------------------------------------------------------------------------
*/
boolean SSEWrapper::isIp(const char* str) {
  DEBUG_SERIAL.print("isIP: ");
  DEBUG_SERIAL.println(str);
  
  for (uint8_t i = 0; str[i]; i++) {
    int c = str[i];
    if (c != '.' && (c < '0' || c > '9')){
      return false;
    }
  }
  return true;
}

/*-----------------------------------------------------------------------------
 * SSEWrapper::getRestart() -- Return if Restart was triggered or not
 * ----------------------------------------------------------------------------
*/
bool SSEWrapper::getRestart(void){
  return restart;
}

/*-----------------------------------------------------------------------------
 * SSEWrapper::setRestart() -- Set if Restart is necessary
 * ----------------------------------------------------------------------------
*/
void SSEWrapper::setRestart(bool _restart){
  restart = _restart;
}
