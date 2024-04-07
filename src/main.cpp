/*************************MinifigLightningSystem Project by Jojo1220**********************************
  Filename:     main.cpp
  Purpose:      main.cpp File for the MinifigLightning System Project
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

/*-----------------------------------------------------------------------------
 * Including of Libary Direcotries
 * ----------------------------------------------------------------------------
*/

#include <Arduino.h>

#include <SSEWrapper.h>

#include <LittleFS.h>

#include <MiniFigLightning.h>

#include <Animation.h>

#ifdef WITH_GDB
  #include <GDBStub.h>
#endif

/*-----------------------------------------------------------------------------
 * Function Headers
 * ----------------------------------------------------------------------------
*/

void incrementStep(uint8_t* _step, bool* _animationRunning);
void initLittleFS(void);
void SSEhandleRequest(AsyncWebServerRequest *request);
void loadRequestArgument2Buffer(AsyncWebServerRequest *request, const char *argName);
void handleUDPHandshake(void);
void RunUpdateOTA(void);
void MD5CalculationFunc(const char* input, char* output);
uint8_t WiFiSetup(uint8_t _variant, bool* _switchMode);

/*-----------------------------------------------------------------------------
 * Setup Global Variables
 * ----------------------------------------------------------------------------
*/
// Neopixel Class Instance as nullptr and dynamic creation during setup because of varable number of LED's
NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod>* strip = nullptr;

// Processstep for Switch/Case
uint8_t step = 0;

// Global Flag to signalize Animation is "still running"
bool animationRunning = false;

// Flag to recognize within Loop if WiFi has to change from e.g. SSE
bool SwitchWiFiMode = true;

// WiFi Mode Glob Save Variable
uint8_t WiFiMode = 0;

// Setup of DNSServer Instance
DNSServer dnsServer;

// Saving OTA Parameters for Update
paramOTA OTAValues;

// OTA State VAriable
ota_state_t _stateOTA = OTA_IDLE;

//WifiClient
WiFiClient client;

// Setup UDP Server Instance
UdpContext udp;

// OTA Global Varialbes to write
uint32_t wirtten, total = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events for SSE Webserver based messages
AsyncEventSource events("/events");

// Adding new Wrapper Functions
SSEWrapper _mySSEWrapper(&server,&events, myHostname); 

//Definition of Global Puffer Array
char requestArgBuffer[32];
char responseBuffer[1024]; // Total need of 1021 in MAX configuration needed!

/*-----------------------------------------------------------------------------
 * Setup Function Running once after Startup
 * ----------------------------------------------------------------------------
*/
void setup() {
  // Dynamic Creation of NeoPixelBus Object
  strip = new NeoPixelBusLg<ConstNeoGrbFeature, ConstNeoEsp8266Uart1Ws2812xMethod> (DefaultNUMPIXELS, PIN);

  //Setup Serial communication
  DEBUG_SERIAL.begin(115200);
  #ifdef WITH_GDB
    DEBUG_SERIAL.println("Starting Debugger Config");
    gdbstub_init();
  #endif
  #ifdef DEBUG_ESP_CORE
    DEBUG_SERIAL.setDebugOutput(true);
  #endif
  DEBUG_SERIAL.print("\n");
  //DEBUG_SERIAL.println("OTA - TESTING");
  DEBUG_SERIAL.println("---Boot---");
  DEBUG_SERIAL.println("MiniFigLightningSystem v1.0.0");
  // Output Information about SDK, Chip and Core Version
  DEBUG_SERIAL_INFORMATION.println(ARDUINO);
  DEBUG_SERIAL_INFORMATION.println(__FILE__);
  DEBUG_SERIAL_INFORMATION.print("---------------------------------------");
  DEBUG_SERIAL_INFORMATION.print("ESP Chip ID: ");
  DEBUG_SERIAL_INFORMATION.print(ESP.getChipId());
  DEBUG_SERIAL_INFORMATION.print("\t Core Version: ");
  DEBUG_SERIAL_INFORMATION.print(ESP.getCoreVersion());
  DEBUG_SERIAL_INFORMATION.print("\t SDK Version: ");
  DEBUG_SERIAL_INFORMATION.print(ESP.getSdkVersion());
  DEBUG_SERIAL_INFORMATION.println("---------------------------------------");

  ESPDebuggingFunction();
  
  //Setup Filesystem
  initLittleFS();

  //Read Out Prferences PROM Data
  systemParameters = readParameters();

  //Initialize Array Elements
  initializeArray(systemParameters.numLEDlenght);

  // pixels begin for NeoPixel LED's
  strip->Begin();

  // Setting Specific Step if Single Animation mode is actvaited -->> Switch Step to xx
  // or Single Color Mode is activated -->> Switch to Step = 99
  if(systemParameters.singleAnimationMode){
    step = systemParameters.AnimationStep;
  }else if(systemParameters.singleColorMode){
    step = 99;
  }

  // Write Brightness of LED's AFTER reading preference Project Settings
  setBrightness(systemParameters.brightness, strip);

  // Updating Number of Pixels to real behaviour by preferences value.
  updateLength(systemParameters.numLEDlenght, strip);

  // Clear whole LED chain to Default "OFF"
  clearALLPixels(strip);

  // Setup mDNS responder
  if (!MDNS.begin(myHostname)) {
    DEBUG_SERIAL.println("[mDNS] Error!");
  } else {
    DEBUG_SERIAL.println("[mDNS] OK!");
    // Add Webserver service to mDNS on Port 80
    MDNS.addService("http", "tcp", 80);
    DEBUG_SERIAL.print("http://");
    DEBUG_SERIAL.print(myHostname);
    DEBUG_SERIAL.println(".local");
    // Add OTA Listener on Port 8266 with/without Passwort
    #ifdef OTAPASSWORT

      DEBUG_SERIAL.println("[OTA] Password needed!");
      MD5CalculationFunc(OTAPASSWORT, OTAValues.password);

      MDNS.enableArduino(OTA_Port, true);
    #else
      MDNS.enableArduino(OTA_Port);
    #endif
  }

  // Setup of UDP Server Instance, Listening on static OTA Port and Setup Callback Function
  if (udp.listen(IP_ADDR_ANY, OTA_Port)){
    DEBUG_SERIAL.println("[UDP] Server Instance started!");
    udp.onRx([](){
      handleUDPHandshake();
    });   
  }
 
  // Adding Custom Event onConnect and send Project Specific Parameter to client
  events.onConnect([](AsyncEventSourceClient *client){
    DEBUG_SERIAL.println("[SSE] onConnect Entered!");
    if(client->lastId()){
      DEBUG_SERIAL.print("[SSE] Client reconnected! Last message Id: ");
      DEBUG_SERIAL.println(client->lastId());
    }
    uint16_t responseLength = snprintf(responseBuffer, sizeof(responseBuffer),   // Total: 113char (without ledAssignment)
      "Hi! SingleColorMode=%s r=%d g=%d b=%d Brigthness=%d Length=%d AnimationMode=%d Speed=%d LedAssignment=",   // 86char's + 1 Null Temrinator (string only)!
      (systemParameters.singleColorMode ? "false" : "true"),      // 5 char
      systemParameters.r, systemParameters.g, systemParameters.b, // 9 char
      systemParameters.brightness, systemParameters.numLEDlenght, // 6 char        
      step, systemParameters.speed);                              // 6 char

    // Fügen Sie die LED-Zuordnungsdaten direkt zum Hauptantwort-Puffer hinzu
    for (uint8_t _i = 0; _i < systemParameters.numLEDlenght; _i++) { // Total: 908 char
        responseLength += snprintf(responseBuffer + responseLength, sizeof(responseBuffer) - responseLength,
          "%d%s", systemParameters.LedwithinEket[_i], (_i < systemParameters.numLEDlenght - 1) ? "," : "");   //253 char ","  + 9 char "1-9" + 180char "10-99" + 465char "100-254"
    }
    DEBUG_SERIAL.println(responseBuffer);

    // send event with message "hi!" + System Specific Parameter for HTML and set reconnect delay to 5 second's
    client->send(responseBuffer, NULL, millis(), 5000);
    
    ESPDebuggingFunction();

  });

  // Setup of SSEWrapper Functions
  _mySSEWrapper.setup(SSEhandleRequest);

  // Start Webserver configuration
  server.begin();
  ESPDebuggingFunction();

  // Setup Wifi connection in AP or STA Mode
  WiFiMode = WiFiSetup(ConnectWiFiSTAMode, &SwitchWiFiMode);

  //Debugging Setup done!
  DEBUG_SERIAL.println("---END-SETUP---");
}

/*-----------------------------------------------------------------------------
 * Loop Function Running continously
 * ----------------------------------------------------------------------------
*/
void loop() {
  // Rewriting LOOP() function to run asynchronosly!

  // Changing WiFi Mode AP/STA during loop because of
  // e.g. SSE Request from Webserver Client
  WiFiMode = WiFiSetup(WiFiMode, &SwitchWiFiMode);

  // Checing within Loop if Number of Pixel has changed e.g. by Websever
  if(strip->PixelCount() != systemParameters.numLEDlenght){
    // Reinitializie Array if it is changed from e.g. SSE -->> newLength
    // has to be called within loop because of WDT Reset during SSE Request and
    // "big" Arrays in combination with LittleFS Access
    updateLength((uint16_t)systemParameters.numLEDlenght, strip);
    initializeArray(systemParameters.numLEDlenght);
    //ledEketLoopCounter(systemParameters.numLEDlenght);
  }
 
  // Whole "true" Animation Loop
  switch(step){
    case 0:
      // Until now "handover-step" can be used in further released as pre-conditions bevor Animations

      //clearALLPixels(&pixels);
      clearALLPixels(strip);
      step = 10;
      break;
    case 10:
      // Fill along the length of the strip in various colors... -->> here: RED
      animationRunning = colorWipe(RgbColor(255,0,0), 80 * systemParameters.speed, strip);
      incrementStep(&step, &animationRunning);
      break;
    case 20:
      // Fill along the length of the strip in various colors... -->> here GREEN
      animationRunning = colorWipe(RgbColor(0, 255,0), 80 * systemParameters.speed, strip);
      incrementStep(&step, &animationRunning);
      break;
    case 30:
      // Fill along the length of the strip in various colors... -->> here BLUE
      animationRunning = colorWipe(RgbColor(0,0,255), 80 * systemParameters.speed, strip);
      incrementStep(&step, &animationRunning);
      break;
    case 40:
      // Do a theater marquee effect in various colors... -->> here WHITE
      animationRunning = theaterChase(RgbColor(127,127,127), 40 * systemParameters.speed, strip);
      incrementStep(&step, &animationRunning);
      break;
    case 50:
       // Do a theater marquee effect in various colors... -->> here Magenta-like
      animationRunning = theaterChase(RgbColor(190,0,170), 40 * systemParameters.speed, strip);
      incrementStep(&step, &animationRunning);
      break;
    case 60:
      // Do a theater marquee effect in various colors... -->> here Cyan-like
      animationRunning = theaterChase(RgbColor(0,170,190), 40 * systemParameters.speed, strip);
      incrementStep(&step, &animationRunning);
      break;
    case 70:
      // Do the Rainbow Animation cycle along the whole strip...
      animationRunning = rainbow(10 * systemParameters.speed, strip);             
      incrementStep(&step, &animationRunning);
      break;
    case 80:
      // Do the Rainbow Animation cycle along the whole strip with theaterChase Variant effect...
      animationRunning = theaterChaseRainbow(10 * systemParameters.speed, strip);
      incrementStep(&step, &animationRunning);
      break;
    case 99:
      // Single Color Animation Mode by Webserver App...
      animationRunning = singleColor(RgbColor(systemParameters.r,systemParameters.g,systemParameters.b), 2 * systemParameters.speed, strip);
      break;
    default:
      // Default step is ALWAYS step 10 and restarting loop (Actually should never happen...)
      step = 10;
      break;
  }

  yield();

  RunUpdateOTA();
}

/*-----------------------------------------------------------------------------
 * Counting-up steps
 * ----------------------------------------------------------------------------
*/
void incrementStep(uint8_t* _step, bool* _animationRunning){
  if(!*_animationRunning && !systemParameters.singleAnimationMode && *_step != 99 && *_step != 100){
    *_step = *_step +10;
  }
}

/*-----------------------------------------------------------------------------
 * SSE Webserver Handler for different kind of Requests
 * ----------------------------------------------------------------------------
*/
void SSEhandleRequest(AsyncWebServerRequest *request) {
  unsigned long deltaT = millis();
  // Watch out:
  // No DELAY() or YIELD() in Callback Functions!!!!
  DEBUG_SERIAL.print("[SSE] Event request ");
  DEBUG_SERIAL.println(request->url().c_str());
  //Reciving blank Request "/" or "/home" -->> Sending index.html File back
  if (strcmp(request->url().c_str(), "/") == 0) {
    if(WiFiMode == StationModeStarted || WiFiMode == ConnectWiFiSTAMode){
      if(LittleFS.exists("/index.html.gz")){
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Cache-Control", "max-age=30");
        request->send(response);

      }else{
        // Send 500 Internal Server Error if index.html cannot be loaded
        request->send(500, "text/plain", "Internal Server Error: Failed to load index file");
      }
    }
    else if(WiFiMode == AccessPointModeStarted || WiFiMode == StartWiFiAPMode){
      if(LittleFS.exists("/apmode.html.gz")){
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/apmode.html.gz", "text/html");
        response->addHeader("Content-Encoding", "gzip");

        // Adding Header for Captive Portal from Homepage
        response->addHeader("Cache-Control", "max-age=30");

        request->send(response);

      }else{
        // Send 500 Internal Server Error if apmode.html cannot be loaded
        request->send(500, "text/plain", "Internal Server Error: Failed to load apmode file");        
      }
    }
    DEBUG_SERIAL_INFORMATION.print("SSEhandleRequest </> [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);
  } 
  else if(strcmp(request->url().c_str(), "/script.js") == 0){
    DEBUG_SERIAL.println("[SSE] JavaScript Reqeuested!");
    if(LittleFS.exists("/script.js.gz")){
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/script.js.gz", "text/javascript");
      response->addHeader("Content-Encoding", "gzip");
      response->addHeader("Cache-Control", "max-age=30");
      request->send(response);

    }else{
      // Send 500 Internal Server Error if script.js cannot be loaded
      request->send(500, "text/plain", "Internal Server Error: Failed to load script file");  
    }
  }
  else if(strcmp(request->url().c_str(), "/style.css") == 0){
    DEBUG_SERIAL.println("[SSE] Style Requested!");
    if(LittleFS.exists("/style.css.gz")){
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/style.css.gz", "text/css");
      response->addHeader("Content-Encoding", "gzip");
      response->addHeader("Cache-Control", "max-age=30");
      request->send(response);

    }else{
      // Send 500 Internal Server Error if style.css cannot be loaded
      request->send(500, "text/plain", "Internal Server Error: Failed to load style file");  
    }
  }
  else if(strcmp(request->url().c_str(), "/images/favicon.png") == 0){
    DEBUG_SERIAL.println("[SSE] Favicon Requested!");
    if(LittleFS.exists("/images/favicon.png")){
      request->send(LittleFS, "/images/favicon.png", "image/png");

    }else{
      // Send 500 Internal Server Error if favicon.png cannot be loaded
      request->send(500, "text/plain", "Internal Server Error: Failed to load favicon file");  
    }
  } 
  else if(strcmp(request->url().c_str(), "/images/git.png") == 0){
    DEBUG_SERIAL.println("[SSE] Git Reqeuested! ");
    if(LittleFS.exists("/images/git.png")){
      request->send(LittleFS, "/images/git.png", "image/png");    

    }else{
      // Send 500 Internal Server Error if git.png cannot be loaded
      request->send(500, "text/plain", "Internal Server Error: Failed to load git file");  
    }
  }
  else if(strcmp(request->url().c_str(), "/images/instruct.png") == 0){
    DEBUG_SERIAL.println("[SSE] Instruct Reqeuested! ");
    if(LittleFS.exists("/images/instruct.png")){
      request->send(LittleFS, "/images/instruct.png", "image/png");    

    }else{
      // Send 500 Internal Server Error if instruct.png cannot be loaded
      request->send(500, "text/plain", "Internal Server Error: Failed to load instruct file");        
    }  
  }//Starting Animation for a single Color Request
  else if(strcmp(request->url().c_str(), "/singleRGBcolor") == 0) {

    if(request->hasArg("RGBvalue")){
      loadRequestArgument2Buffer(request, "RGBvalue");

      DEBUG_SERIAL.print("RGB Code Recived: ");
      DEBUG_SERIAL.println(requestArgBuffer);

      // Find beginning Position of "(", "(" + 1, to "(" overjump
      uint8_t startPos = strchr(requestArgBuffer, '(') - requestArgBuffer + 1;

      // Find endposition of ",", while searching for ",", starting at "startPos"
      uint8_t comma1Pos = strchr(requestArgBuffer + startPos, ',') - requestArgBuffer;

      // Find endposition of ",", while searching for ",", stating at "comma1Pos + 1"
      uint8_t comma2Pos = strchr(requestArgBuffer + comma1Pos + 1, ',') - requestArgBuffer;

      // Splitting up to r, g, and b
      systemParameters.r = atoi(requestArgBuffer + startPos);
      systemParameters.g = atoi(requestArgBuffer + comma1Pos + 1);
      systemParameters.b = atoi(requestArgBuffer + comma2Pos + 1);

      // Writing recived Values back to Preferences for Power off/on
      preferencesBegin();
      preferences.putUChar(RED_KEY, systemParameters.r);
      preferences.putUChar(GREEN_KEY, systemParameters.g);
      preferences.putUChar(BLUE_KEY, systemParameters.b);
      preferences.end();
    }
    // Response to Client "OK"
    request->send(200, "text/plain", "OK");
    DEBUG_SERIAL_INFORMATION.print("SSEhandleRequest </singleRGBcolor> [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);

  } // Activating single color mode
  else if(strcmp(request->url().c_str(), "/singleColor") == 0){
    if(request->hasArg("SingleColorMode"))
    {
      loadRequestArgument2Buffer(request, "SingleColorMode");

      DEBUG_SERIAL.print("SingleColorMode: ");
      DEBUG_SERIAL.println(requestArgBuffer);
      if(strcmp(requestArgBuffer, "true")){
        // If Single Colormode is/was Activated --> Restart Loop Animations
        if(step == 99){
          step = 0;
          
          systemParameters.singleColorMode = false;
          systemParameters.singleAnimationMode = false;

          // Writing recived Values back to Preferences for Power off/on
          preferencesBegin();
          preferences.putBool(SINGLECOLORMODE_KEY, false);
          preferences.putBool(SINGLEANIMATIONMODE_KEY, false);
          preferences.putUChar(SINGLEANIMATIONSTEP_KEY, 0);
          preferences.putUChar(RED_KEY, DefaultRed);
          preferences.putUChar(GREEN_KEY, DefaultGreen);
          preferences.putUChar(BLUE_KEY, DefaultBlue);
          preferences.end();

        }
      }// Activate Single Color Mode 99
      else if(strcmp(requestArgBuffer, "false")){
        step = 99;

        systemParameters.singleColorMode = true;
        systemParameters.singleAnimationMode = false;

        // Writing recived Values back to Preferences for Power off/on
        preferencesBegin();
        preferences.putBool(SINGLECOLORMODE_KEY, true);
        preferences.putBool(SINGLEANIMATIONMODE_KEY, false);
        preferences.putUChar(SINGLEANIMATIONSTEP_KEY, 0);
        preferences.putUChar(RED_KEY, systemParameters.r);
        preferences.putUChar(GREEN_KEY, systemParameters.g);
        preferences.putUChar(BLUE_KEY, systemParameters.b);
        preferences.end();
      }
    }
    // Response to Client "OK"
    request->send(200, "text/plain", "OK");
    DEBUG_SERIAL_INFORMATION.print("SSEhandleRequest </singleColor> [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);

  }// Activating Single Animation Mode
  else if(strcmp(request->url().c_str(), "/animationMode") == 0){
    
    if(request->hasArg("animationModeChecked")){
      loadRequestArgument2Buffer(request, "animationModeChecked");
      DEBUG_SERIAL.print("animationModeChecked: ");
      DEBUG_SERIAL.println(requestArgBuffer);

      // Deactivating Single Color Mode if it is/was active and save its defaults!
      if(step == 99 || atoi(requestArgBuffer) == 0){ 
          step = 0;
          
          systemParameters.singleColorMode = false;
          systemParameters.singleAnimationMode = false;

        // Writing recived Values back to Preferences for Power off/on
          preferencesBegin();
          preferences.putBool(SINGLECOLORMODE_KEY, false);
          preferences.putBool(SINGLEANIMATIONMODE_KEY, false);
          preferences.putUChar(SINGLEANIMATIONSTEP_KEY, 0);
          preferences.putUChar(RED_KEY, DefaultRed);
          preferences.putUChar(GREEN_KEY, DefaultGreen);
          preferences.putUChar(BLUE_KEY, DefaultBlue);
          preferences.end();
           
      }
      // If Animation was Triggered --> Save step Number and Animation mode back!
      if(atoi(requestArgBuffer) != 0){  
        // Set Recived Dropdown-value to Animation Step!
        step = atoi(requestArgBuffer);

        systemParameters.singleColorMode = false;
        systemParameters.singleAnimationMode = true;

        // Writing recived Values back to Preferences for Power off/on
        preferencesBegin();
        preferences.putBool(SINGLECOLORMODE_KEY, false);
        preferences.putBool(SINGLEANIMATIONMODE_KEY, true);
        preferences.putUChar(SINGLEANIMATIONSTEP_KEY, step);
        preferences.putUChar(RED_KEY, DefaultRed);
        preferences.putUChar(GREEN_KEY, DefaultGreen);
        preferences.putUChar(BLUE_KEY, DefaultBlue);
        preferences.end();    
      }
    }
    // Response to Client "OK"
    request->send(200, "text/plain", "OK");
    DEBUG_SERIAL_INFORMATION.print("SSEhandleRequest </animationMode> [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);

  } //Updating Brigntess Value if reciving: -->> Also Saving within Preferences!
  else if(strcmp(request->url().c_str(), "/brightness") == 0){

    if(request->hasArg("Brightness")){
      loadRequestArgument2Buffer(request, "Brightness");
      DEBUG_SERIAL.print("Brightness Recived: ");
      DEBUG_SERIAL.println(requestArgBuffer);

      systemParameters.brightness =  atoi(requestArgBuffer);

      setBrightness(systemParameters.brightness, strip);

      // Writing recived Values back to Preferences for Power off/on
      preferencesBegin();
      preferences.putUChar(BRIGHTNESS_KEY,systemParameters.brightness);
      preferences.end(); 
    }
    // Response to Client "OK"
    request->send(200, "text/plain", "OK");
    DEBUG_SERIAL_INFORMATION.print("SSEhandleRequest </brightness> [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);

  }// Update Animation Speed
  else if(strcmp(request->url().c_str(), "/updateSpeed") == 0){
    if(request->hasArg("Speed")){
      loadRequestArgument2Buffer(request, "Speed");
      DEBUG_SERIAL.print("Speed Recived: ");
      DEBUG_SERIAL.println(requestArgBuffer);

      systemParameters.speed =  atoi(requestArgBuffer);

      // Writing recived Values back to Preferences for Power off/on
      preferencesBegin();
      preferences.putUChar(SPEED_KEY,systemParameters.speed);
      preferences.end(); 
    }
    // Response to Client "OK"
    request->send(200, "text/plain", "OK");
    DEBUG_SERIAL_INFORMATION.print("SSEhandleRequest </updateSpeed> [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);

  }//Adapt WS2812b LED length if Chain is adapted in Length
  else if(strcmp(request->url().c_str(), "/updateLEDLength") == 0){
    
    if(request->hasArg("newLength")){
      loadRequestArgument2Buffer(request, "newLength");
      DEBUG_SERIAL.print("newLength: ");
      DEBUG_SERIAL.println(requestArgBuffer);

      systemParameters.numLEDlenght = atoi(requestArgBuffer);

      // Writing recived Values back to Preferences for Power off/on
      preferencesBegin();
      preferences.putUChar(LEDLENGTH_KEY, systemParameters.numLEDlenght);
      preferences.end();

    }
    // Response to Client "OK"
    request->send(200, "text/plain", "OK");
    DEBUG_SERIAL_INFORMATION.print("SSEhandleRequest </updateLEDLength> [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);

  }// Update Specific LED Assignment
  else if(strcmp(request->url().c_str(), "/updateSpecLED") == 0){

    if (request->hasArg("LedNr") && request->hasArg("Assig")) {
      loadRequestArgument2Buffer(request, "LedNr");
      uint8_t _LedNr = atoi(requestArgBuffer);
      DEBUG_SERIAL.print("LED Nr. : ");
      DEBUG_SERIAL.print(_LedNr);

      loadRequestArgument2Buffer(request, "Assig");
      uint8_t _Assignet = atoi(requestArgBuffer);

      DEBUG_SERIAL.print(" Assignet to EKET Nr. ");
      DEBUG_SERIAL.println(_Assignet);

      systemParameters.LedwithinEket[_LedNr] = _Assignet;

      snprintf(requestArgBuffer,sizeof(requestArgBuffer), "%s%d", LEDASSIGNMENT_KEY, _LedNr);

      // Writing recived Values back to Preferences for Power off/on
      preferencesBegin();
      preferences.putUChar(requestArgBuffer,_Assignet);
      preferences.end();
    }
    // Response to Client "OK"
    request->send(200, "text/plain", "OK");

    DEBUG_SERIAL_INFORMATION.print("SSEhandleRequest </updateSpecLED> [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);

  }//Save WiFi Crednetials back to Prferences NVS
  else if (strcmp(request->url().c_str(), "/savePreferences") == 0) {

    //Update WiFi Credentials for STA Mode
    if (request->hasArg("ssid") && request->hasArg("password")) {
      loadRequestArgument2Buffer(request, "ssid");
      DEBUG_SERIAL.print("SSID Updated: ");
      DEBUG_SERIAL.print(requestArgBuffer);
      snprintf(systemParameters.ssid, sizeof(systemParameters.ssid), "%s", requestArgBuffer);

      loadRequestArgument2Buffer(request, "password");
      DEBUG_SERIAL.print(" Password Updated: ");
      DEBUG_SERIAL.println(requestArgBuffer);
      snprintf(systemParameters.password, sizeof(systemParameters.password), "%s", requestArgBuffer);

      SwitchWiFiMode = true;
      WiFiMode = ConnectWiFiSTAMode;

      // Writing recived Values back to Preferences for Power off/on
      preferencesBegin();
      preferences.putString(SSID_KEY, systemParameters.ssid);
      preferences.putString(PASSWORD_KEY, systemParameters.password);
      preferences.end();

      //request->send(200, "text/plain", "Network updated!");

      //redirect Client to http://<ESPHostname>.local and then Close connection during Changing WiFi-Mode
      snprintf(requestArgBuffer, sizeof(requestArgBuffer), "http://%s", "8.8.8.8");
      request->redirect(requestArgBuffer);
      request->client()->close();
      DEBUG_SERIAL_INFORMATION.print("SSEhandleRequest </savePreferences> <OK> [ms]: ");
      DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);
      return;
    }
    // Response to Client "OK"
    request->send(200, "text/plain", "OK");

    DEBUG_SERIAL_INFORMATION.print("SSEhandleRequest </savePreferences> <NOK> [ms]: ");
    DEBUG_SERIAL_INFORMATION.println(millis()-deltaT);

  }
};

/*-----------------------------------------------------------------------------
 * WiFi Setup Function in Station Mode or Access Point mode depending on Credentials
 * ----------------------------------------------------------------------------
*/
uint8_t WiFiSetup(uint8_t _variant, bool* _switchMode){

  if(*_switchMode && (_variant == ConnectWiFiSTAMode || _variant == StartWiFiAPMode)){
    ESPDebuggingFunction();

    DEBUG_SERIAL.println("[WiFi] Mode Changing!");
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.disconnect(true);  // Dicsonnect and start "from the Beginning"
    WiFi.softAPdisconnect(true); // Function will set currently configured SSID and password of the soft-AP to null values. The parameter  is optional. If set to true it will switch the soft-AP mode off.
    dnsServer.stop();       // Stop DNS Service
    DEBUG_SERIAL.println("[WiFi] Services close");

    // Connecting ESP8266 as Station Mode
    if(_variant == ConnectWiFiSTAMode){
      DEBUG_SERIAL.println("[WiFi] STA Mode!");
      WiFi.mode(WIFI_STA);
      // Waiting until mode is REAL changed!
      unsigned long start = millis(); 
      while(millis() - start < 5000UL){
        if(WiFi.getMode() != WIFI_STA){
          yield();
        }else{
          DEBUG_SERIAL.println("[WiFi] STA Mode Setup - OK");
          break;
        }
      }
      WiFi.persistent(false);
      WiFi.begin(systemParameters.ssid, systemParameters.password);
      WiFi.config(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0),
                      IPAddress(AP_IP_ADDRESS), IPAddress(DNS_IP_ADDRESS));

      // Retry Connectiong ESP8266 in STA Mode, if it fails start Fallback AP mode
      for(uint8_t _i = 0; _i< 10; _i++) {
        if (WiFi.status() == WL_CONNECTED) {
          DEBUG_SERIAL.print("[WiFi] IP address: ");
          DEBUG_SERIAL.println(WiFi.localIP());

          MDNS.notifyAPChange();     //restart MDNS Service

          DEBUG_SERIAL.println("[WiFi] Services restarted");

          WiFi.scanDelete();

          *_switchMode = false;
          ESPDebuggingFunction();
          return StationModeStarted; // Successfull connected -->> return!
        }
        DEBUG_SERIAL.println(_i + 1);
        for (unsigned long start = millis(); millis() - start < 2000UL;) {
          yield(); // "yield()-based-break for 2seconds between reconnect"
        }      
      }

      if(WiFi.status() != WL_CONNECTED){
        DEBUG_SERIAL.println("[WiFi] STA not possible - starting AP!");
        WiFi.disconnect(true);  // Dicsonnect and start "from the Beginning"
        WiFi.softAPdisconnect(true); // Function will set currently configured SSID and password of the soft-AP to null values. The parameter  is optional. If set to true it will switch the soft-AP mode off.
        WiFi.mode(WIFI_OFF);
        // Waiting until mode is REAL changed!
        unsigned long start = millis();
        while(millis() - start < 5000UL){
          if(WiFi.getMode() != WIFI_OFF){
            yield();
          }else{
            DEBUG_SERIAL.println("[WiFi] OFF Setup - OK");
            break;
          }
        }
        _variant = StartWiFiAPMode;
      }
    }
    // Connecting/starting ESP8266 as/in AccessPointMode
    if (_variant == StartWiFiAPMode) {
      DEBUG_SERIAL.println("[WiFi] AP Mode!");

      WiFi.mode(WIFI_AP);
      // Waiting until mode is REAL changed!
      unsigned long start = millis(); 
      while(millis() - start < 5000UL){
        if(WiFi.getMode() != WIFI_AP){
          yield();
        }else{
          DEBUG_SERIAL.println("[WiFi] AP Mode Setup - OK");
          break;
        }
      }
      WiFi.persistent(false);
      WiFi.softAPConfig(IPAddress(AP_IP_ADDRESS),IPAddress(AP_IP_ADDRESS),IPAddress(SUBNET_MASK));
      WiFi.softAP(APSSID, APPSK);

      for (unsigned long start = millis(); millis() - start < 2000UL;) {
        yield(); // "yield()-based-break for 1seconds bevor Starting the SoftAP"
      }

      DEBUG_SERIAL.println("------------------[WiFi Info]--------------");
      DEBUG_SERIAL.print("[WiFi] IP Address: ");
      DEBUG_SERIAL.print(IPAddress(WiFi.softAPIP()));
      DEBUG_SERIAL.print("\n");
      DEBUG_SERIAL.print(WiFi.softAPSSID());
      DEBUG_SERIAL.print("\t");
      DEBUG_SERIAL.print(WiFi.softAPPSK());
      DEBUG_SERIAL.print("\n");
      DEBUG_SERIAL.print("Channel: ");
      DEBUG_SERIAL.print(WiFi.channel());
      DEBUG_SERIAL.print("\t Physical Mode: ");
      DEBUG_SERIAL.print(WiFi.getPhyMode());
      DEBUG_SERIAL.println("\n---------------------------------------------\n");

      // Scanning for Netowrks to avoid "First Scan" from AP-Mode to leave empty!
      WiFi.scanNetworks(true, true);

      MDNS.notifyAPChange();     //restart MDNS Service

      // Startinng DNSServer on Port 53
      dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
      dnsServer.start(53, "*", WiFi.softAPIP());

      DEBUG_SERIAL.println("[WiFi] Services restarted");

      *_switchMode = false;
      ESPDebuggingFunction();
      return  AccessPointModeStarted;

    };
  }else if(*_switchMode && _variant != ConnectWiFiSTAMode && _variant != StartWiFiAPMode){
    DEBUG_SERIAL.println("[WiFi] Mode selected is NOT supported!");
  };

  //For Mein-Loop Function: Update dnsServer and MDNS Service!
    dnsServer.processNextRequest();
    MDNS.update();

  return _variant;
}

/*-----------------------------------------------------------------------------
 * Initialize LittleFS
 * ----------------------------------------------------------------------------
*/
void initLittleFS(void) {
  if (!LittleFS.begin()) {
    DEBUG_SERIAL.println("[LittleFS] error!");
  }
  else{
    DEBUG_SERIAL.println("[LittleFS] mounted!");
  }
}

/*-----------------------------------------------------------------------------
 * Load Request Argument from WebserverRequest to temp Buffer
 * ----------------------------------------------------------------------------
*/
void loadRequestArgument2Buffer(AsyncWebServerRequest *request, const char *argName){
  if(request->hasArg(argName)){
    strncpy(requestArgBuffer, request->arg(argName).c_str(), sizeof(requestArgBuffer) - 1);
    requestArgBuffer[sizeof(requestArgBuffer) - 1] = '\0'; // Buffer has always a null terminating zero
  } else {
    requestArgBuffer[0] = '\0'; // Set buffer to an empty string if the argument is not present
  }
}

/*-----------------------------------------------------------------------------
 * UDP Callback Handle - Handshake 4 OTA Update
 * ----------------------------------------------------------------------------
*/
void handleUDPHandshake(void){
  if (!udp.next()){
    return;
  }
  if (udp.getSize()){
    bool authentificationSuccess;
    char buffer[128];
    udp.read(buffer, sizeof(buffer));

    // Serial Print Buffer Content recived by UDP Connection
    DEBUG_SERIAL.print("[UDP] Recived Message: ");
    DEBUG_SERIAL.println(buffer);

    // Check Package Request and save informations like Command, Port, Content Size and md5 Checksum
    // Also Check, if Authentification by password is necessary or not!
    if (_stateOTA == OTA_IDLE){
      authentificationSuccess = false;
      sscanf(buffer, "%d %d %d %32s", &OTAValues.command, &OTAValues.localPort, &OTAValues.contentSize, OTAValues.md5CheckSum);

      DEBUG_SERIAL.print("[UDP] Command: ");
      DEBUG_SERIAL.print(OTAValues.command);
      DEBUG_SERIAL.print(" LocalPort: ");
      DEBUG_SERIAL.print(OTAValues.localPort);
      DEBUG_SERIAL.print(" ContentSize [Kb]: ");
      DEBUG_SERIAL.print(OTAValues.contentSize);
      DEBUG_SERIAL.print(" md5Checksum: ");
      DEBUG_SERIAL.println(OTAValues.md5CheckSum);

      OTAValues.IP = udp.getRemoteAddress();
      OTAValues.port = udp.getRemotePort();

      // Only Excecute Authentification Part of Code if OTA Password is set or not!
      #ifdef OTAPASSWORT
        char _micros[33];
        snprintf(_micros, sizeof(_micros), "%lu", micros());
        MD5CalculationFunc(_micros, OTAValues.nonce);

        char auth_req[38];
        sprintf(auth_req, "AUTH %s", OTAValues.nonce);
        DEBUG_SERIAL.print("Authorisation String: ");
        DEBUG_SERIAL.println(auth_req);
        udp.append((const char *)auth_req, strlen(auth_req));
        udp.send(OTAValues.IP, OTAValues.port);
        udp.flush();
        _stateOTA = OTA_WAITAUTH;
        return;
      } // If Password is set introduce state step to wait bevor enable Upload
      else if (_stateOTA == OTA_WAITAUTH){
        DEBUG_SERIAL.println("[UDP] Waiting for Authentification");
        int Authcmd = 0;
        char md5[33];
        char md_5[33];
        sscanf(buffer, "%d %32s %32s", &Authcmd, md5, md_5);

        DEBUG_SERIAL.print("[UDP] Command: ");
        DEBUG_SERIAL.print(Authcmd);
        DEBUG_SERIAL.print(" MD5 hash:");
        DEBUG_SERIAL.print(md5);
        DEBUG_SERIAL.print(" MD_5 hash: ");
        DEBUG_SERIAL.println(md_5);

        // After Checking basic requirements like Authentification Command and md5 len. 32 chars, go on
        // checking md5 hash strings if correct or not
        if (Authcmd == U_AUTH && strlen(md5) == 32 && strlen(md_5) == 32)
        {
          DEBUG_SERIAL.print("Prerequirments OK - Checking passwort hash: ");
          char challenge[99];
          char result[33]; 
          snprintf(challenge, sizeof(challenge), "%s:%s:%s", OTAValues.password, OTAValues.nonce, md5);
          DEBUG_SERIAL.println(challenge);
          MD5CalculationFunc(challenge, result);

          if (strcmp(result, md_5) == 0){
            DEBUG_SERIAL.println("MD5 Calculation OK - Run Update!");
            authentificationSuccess = true;
            _stateOTA = OTA_RUNUPDATE;
          } else {
            _stateOTA = OTA_IDLE;
            DEBUG_SERIAL.println("MD5 Calculation failed - Return ERR!");
            udp.append("Authentification Failed", 21);
            udp.send(OTAValues.IP, OTAValues.port);
          }
        } // If basic requirments not given -- return!
        else {
          DEBUG_SERIAL.println("Authentification failed!");
          _stateOTA = OTA_IDLE;
          udp.append("Authentification Failed", 21);
          udp.send(OTAValues.IP, OTAValues.port);
          udp.listen(IP_ANY_TYPE, OTA_Port);
          return;
        }
      
      #else
        // Directly go on with upload if prerequirmenets are given and NO password check!
        authentificationSuccess = true;
      #endif
    }
    // [UDP] md5Checksum: cmd // Port // Filesize // Checksum
    if (strlen(OTAValues.md5CheckSum) == 32 && _stateOTA != OTA_WAITAUTH && authentificationSuccess && (OTAValues.command == U_FLASH || OTAValues.command == U_FS)){
      DEBUG_SERIAL.println("[UDP] OTA request received");
      udp.flush();

      Update.runAsync(true);
      if (!Update.begin(OTAValues.contentSize, OTAValues.command)){
        DEBUG_SERIAL.println("[OTA] Begin Error");
        #ifdef SERIAL_OUTPUT_DEBUGGING
          Update.printError(Serial);
        #endif
        udp.append("ERR: ", 5);
        udp.send(OTAValues.IP, OTAValues.port);
        _stateOTA = OTA_IDLE;
        udp.listen(IP_ANY_TYPE, OTA_Port);
      }

      delay(20);

      // Confirm Successfull Initiated OTA Updated back to Host
      udp.append("OK", 2);
      udp.send(OTAValues.IP, OTAValues.port);

      Update.setMD5(OTAValues.md5CheckSum);

      _stateOTA = OTA_RUNUPDATE;

      udp.flush();
    }
    else{
      if (_stateOTA != OTA_WAITAUTH){
        _stateOTA = OTA_IDLE;
        udp.listen(IP_ANY_TYPE, OTA_Port);
      }
    }
  }
}

/*-----------------------------------------------------------------------------
 * RunUpdateOTA() - True Loop Update function
 * ----------------------------------------------------------------------------
*/
void RunUpdateOTA(void){
  if (_stateOTA == OTA_RUNUPDATE){
    if (!client.connect(OTAValues.IP, OTAValues.localPort)){
      DEBUG_SERIAL.println("[OTA] Client Connection to Host failed!");
      _stateOTA = OTA_IDLE;
      wirtten = 0;
      total = 0;

      udp.flush();
      udp.listen(IP_ANY_TYPE, OTA_Port);

      client.flush();
      client.stop();

      return;
    }

    client.setNoDelay(true);

    while (!Update.isFinished() && (client.connected() || client.available())){
      int waited = 1000;
      while (!client.available() && waited--){
        delay(3);
        yield();
        if (!waited){
          DEBUG_SERIAL.println("[OTA] Timeout - no package!");
          udp.listen(IP_ADDR_ANY, OTA_Port);
          return;
        }
      }

      wirtten = Update.write(client);
      delay(10);

      if (wirtten > 0){
        client.print(wirtten, DEC);
        total += wirtten;
        size_t temp_size = (Update.progress() * 100) / Update.size();
        DEBUG_SERIAL.print("[OTA] Progress: ");
        DEBUG_SERIAL.println(temp_size);
        client.print(temp_size, DEC);
      }

      if (Update.hasError()){
        DEBUG_SERIAL.print("[OTA] Updated failed: ");
        DEBUG_SERIAL.println(Update.getErrorString());
      }
      yield();
    }

    if (!Update.end(true)){
      DEBUG_SERIAL.print("[OTA] Updated end with: ");
      DEBUG_SERIAL.println(Update.getErrorString());
      client.print(Update.getErrorString());
      client.flush();
      client.stop();
    }
    else{
      DEBUG_SERIAL.println("[OTA] finished - RESTART");
      DEBUG_SERIAL.println("Reboot!");
      client.flush();
      delay(2000);
      // Confirm OK back to Host
      client.print("OK");
      client.flush();
      client.stop();
      _mySSEWrapper.setRestart(true);
    }
  }

  if(_mySSEWrapper.getRestart()){
      // Deactivating WiFi 
      WiFi.mode(WIFI_OFF);

      // Wait 1s until reboot
      delay(2000);
      // ESP8266 Restart after Update
      // Using reset() Methode instead of restart() because with restart there is no "real" restart of ESP8266!
      ESP.restart();
  }
}

/*-----------------------------------------------------------------------------
 * MD5Calculation Function
 * ----------------------------------------------------------------------------
*/
void MD5CalculationFunc(const char *input, char *output){
  MD5Builder md5Builder;
  md5Builder.begin();
  md5Builder.add(input);
  md5Builder.calculate();
  md5Builder.getChars(output);
}
