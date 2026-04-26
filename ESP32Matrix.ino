#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDSprites.h>
#include <LEDText.h>
#include <FontMatrise.h>
#include <Preferences.h>
#include "html.h"
#include "config.h"
#include "secrets.h"

#define OTA_HOSTNAME   "esp32matrix"
#define OTA_PASSWORD   "matrix-ota"

#define LED_PIN        26
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B
#define MATRIX_WIDTH   16
#define MATRIX_HEIGHT  16
#define MATRIX_TYPE    HORIZONTAL_ZIGZAG_MATRIX
#define DEFAULT_BRIGHTNESS   128
#define DEFAULT_MILLIAMPS 2000
#define DEFAULT_VOLTS     5

#define APP_MENU -1
#define APP_TETRIS 0
#define APP_SNAKE 1
#define APP_BREAKOUT 2
#define APP_PIXEL 3

cLEDText ScrollingMsg;

AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

int currentApp = APP_MENU;

// Persistent config (saved to NVS via Preferences)
Preferences prefs;
uint8_t  cfgBrightness = DEFAULT_BRIGHTNESS;
uint16_t cfgMilliamps  = DEFAULT_MILLIAMPS;
uint8_t  cfgVolts      = DEFAULT_VOLTS;

void loadConfig() {
  prefs.begin("matrix", true); // read-only
  cfgBrightness = prefs.getUChar("brightness",  DEFAULT_BRIGHTNESS);
  cfgMilliamps  = prefs.getUShort("milliamps",  DEFAULT_MILLIAMPS);
  cfgVolts      = prefs.getUChar("volts",       DEFAULT_VOLTS);
  prefs.end();
}

void saveConfig() {
  prefs.begin("matrix", false);
  prefs.putUChar("brightness", cfgBrightness);
  prefs.putUShort("milliamps", cfgMilliamps);
  prefs.putUChar("volts",      cfgVolts);
  prefs.end();
}

cLEDMatrix<MATRIX_WIDTH, -MATRIX_HEIGHT, MATRIX_TYPE> leds;

enum btnInput {NONE, UP, DOWN, LEFT, RIGHT, MENU};
btnInput currentInput = NONE;

#include "Tetris.h"
#include "Menu.h"
#include "Snake.h"
#include "Breakout.h"
#include "Pixel.h"

Menu menu = Menu();
Pixel pixel = Pixel();
Snake snake = Snake();
Breakout breakout = Breakout();
Tetris tetris = Tetris();

void setup()
{
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  showIP();

  // Web server to serve HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", htmlPage);
  });

  // Config page
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", configPage);
  });

  // Config API – GET returns current values as JSON
  server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"brightness\":" + String(cfgBrightness) +
                  ",\"milliamps\":"  + String(cfgMilliamps)  +
                  ",\"volts\":"      + String(cfgVolts)      + "}";
    request->send(200, "application/json", json);
  });

  // Config API – POST applies and saves new values
  server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("brightness", true)) {
      cfgBrightness = (uint8_t)constrain(request->getParam("brightness", true)->value().toInt(), 0, 255);
    }
    if (request->hasParam("milliamps", true)) {
      cfgMilliamps = (uint16_t)constrain(request->getParam("milliamps", true)->value().toInt(), 100, 5000);
    }
    if (request->hasParam("volts", true)) {
      cfgVolts = (uint8_t)constrain(request->getParam("volts", true)->value().toInt(), 3, 5);
    }
    FastLED.setBrightness(cfgBrightness);
    FastLED.setMaxPowerInVoltsAndMilliamps(cfgVolts, cfgMilliamps);
    saveConfig();
    String json = "{\"ok\":true,\"brightness\":" + String(cfgBrightness) +
                  ",\"milliamps\":"  + String(cfgMilliamps)  +
                  ",\"volts\":"      + String(cfgVolts)      + "}";
    request->send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Web server started");

  // WebSocket setup
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("WebSocket server started");

  setupOTA();

  loadConfig();
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setMaxPowerInVoltsAndMilliamps(cfgVolts, cfgMilliamps);
  FastLED.setBrightness(cfgBrightness);
  FastLED.clear(true);
  FastLED.show();
}

void setupOTA(){
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    FastLED.clear(true);
    FastLED.show();
    Serial.println("OTA update starting");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA update complete");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // Render a progress bar across the 16x16 matrix (one row fills every ~6%).
    uint16_t totalPixels = MATRIX_WIDTH * MATRIX_HEIGHT;
    uint16_t filled = (uint32_t)progress * totalPixels / (total ? total : 1);
    FastLED.clear();
    for (uint16_t i = 0; i < filled; i++) {
      leds(i % MATRIX_WIDTH, i / MATRIX_WIDTH) = CRGB::Green;
    }
    FastLED.show();
    Serial.printf("OTA progress: %u%%\r", (progress * 100) / (total ? total : 1));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA error [%u]: ", error);
    if (error == OTA_AUTH_ERROR)         Serial.println("auth failed");
    else if (error == OTA_BEGIN_ERROR)   Serial.println("begin failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("connect failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("receive failed");
    else if (error == OTA_END_ERROR)     Serial.println("end failed");
  });

  ArduinoOTA.begin();
  Serial.printf("OTA ready: %s.local\n", OTA_HOSTNAME);
}

void showIP(){
  char strIP[16] = "               ";
  IPAddress ip = WiFi.localIP();
  ip.toString().toCharArray(strIP, 16);
  Serial.println(strIP);
  ScrollingMsg.SetFont(MatriseFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);
  ScrollingMsg.SetText((unsigned char *)strIP, sizeof(strIP) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0xff, 0xff);
  ScrollingMsg.SetScrollDirection(SCROLL_LEFT);
  ScrollingMsg.SetFrameRate(160 / MATRIX_WIDTH);       // Faster for larger matrices

  while(ScrollingMsg.UpdateText() == 0) {
    FastLED.show();  
  }
}

void loop()
{
  Serial.println("In the main program loop()");
  Serial.printf("currentApp: %d\n", currentApp);
  switch (currentApp) {
    case APP_MENU : runMenu();
    break;
    case APP_TETRIS : runTetris();
    break;
    case APP_SNAKE : runSnake();
    break;
    case APP_BREAKOUT : runBreakout();
    break;
    case APP_PIXEL : runPixel();
  }
}

void runMenu(){
  bool isRunning = true;
  Serial.println("In runMenu() about to call setup()");
  Serial.printf("currentApp in runMenu(): %d\n", currentApp);
  menu.setup();
  while (isRunning) {
    ArduinoOTA.handle();
    isRunning = menu.loop();
  }
  Serial.println("About to exit runMenu()");
  Serial.printf("currentApp in runMenu(): %d\n", currentApp);
}

void runTetris(){
  bool isRunning = true;
  Serial.println("In runTetris() about to call setup()");
  Serial.printf("currentApp in runTetris(): %d\n", currentApp);
  tetris.setup();
  while (isRunning) {
    ArduinoOTA.handle();
    isRunning = tetris.loop();
  }
  Serial.println("About to exit runTetris()");
  Serial.printf("currentApp in runTetris(): %d\n", currentApp);
}

void runSnake(){
  bool isRunning = true;
  Serial.println("In runSnake() about to call setup()");
  Serial.printf("currentApp in runSnake(): %d\n", currentApp);
  snake.setup();
  while (isRunning) {
    ArduinoOTA.handle();
    isRunning = snake.loop();
  }
  Serial.println("About to exit runSnake()");
  Serial.printf("currentApp in runSnake(): %d\n", currentApp);
}

void runBreakout(){
  bool isRunning = true;
  Serial.println("In runBreakout() about to call setup()");
  Serial.printf("currentApp in runBreakout(): %d\n", currentApp);
  breakout.setup();
  while (isRunning) {
    ArduinoOTA.handle();
    isRunning = breakout.loop();
  }
  Serial.println("About to exit runBreakout()");
  Serial.printf("currentApp in runBreakout(): %d\n", currentApp);
}

void runPixel(){
  bool isRunning = true;
  Serial.println("In runPixel() about to call setup()");
  Serial.printf("currentApp in runPixel(): %d\n", currentApp);
  pixel.setup();
  while (isRunning) {
    ArduinoOTA.handle();
    isRunning = pixel.loop();
  }
  Serial.println("About to exit runPixel()");
  Serial.printf("currentApp in runPixel(): %d\n", currentApp);
}

// WebSocket event handling
void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t *payload, size_t length) {
    if (type == WStype_TEXT) {
        Serial.print("Received message from client ");
        Serial.print(client_num);
        Serial.print(": ");
        Serial.println((char)payload[0]);
        Serial.print("Current App: ");
        Serial.println(currentApp);

        // General handling of the game controls 
        switch ((char)payload[0]) {
            case 'w': 
                currentInput = UP; 
                Serial.println("Input: UP");
                break;
            case 'a': 
                currentInput = LEFT; 
                Serial.println("Input: LEFT");
                break;
            case 's': 
                currentInput = DOWN; 
                Serial.println("Input: DOWN");
                break;
            case 'd': 
                currentInput = RIGHT; 
                Serial.println("Input: RIGHT");
                break;
            case 'm':
                currentInput = MENU;
                Serial.println("Input: MENU");
                break;
            case 'r':
                currentInput = NONE;
                if      (currentApp == APP_SNAKE)    snake.exitSnake();
                else if (currentApp == APP_TETRIS)   tetris.exitTetris();
                else if (currentApp == APP_BREAKOUT) breakout.exitBreakout();
                Serial.println("Input: RESET");
                break;
            default:
                Serial.println("Unknown input received");
                break;
        }
        Serial.println("Break from switch statement");

        if (currentApp == APP_MENU) {
          Serial.println("Handling input for Menu");
          if (currentInput == UP) {
            int m = menu.getMenuItem();
            m = ++m % 4;
            menu.setMenuItem(m);
            menu.menuChanged(m);
          }
          if (currentInput == DOWN) {
            int m = menu.getMenuItem();
            if(--m < 0 ) m = 3;
            menu.setMenuItem(m);
            menu.menuChanged(m);
          }
          if (currentInput == RIGHT) {
            int m = menu.getMenuItem();
            currentApp = m;
            menu.exitMenu();
          }
        } 
        else if (currentApp == APP_PIXEL) {
          Serial.println("Handling input for Pixel Art");
          if (currentInput == MENU) {
            currentApp = APP_MENU;
            pixel.exitPixelArt();
          }
        }
        else if (currentApp == APP_SNAKE) {
          Serial.println("Handling input for Snake");
          if (currentInput == MENU) {
            currentApp = APP_MENU;
            snake.exitSnake();
          }
          else if (snake.isPaused()){
            snake.reset();
          }
        }
        else if (currentApp == APP_BREAKOUT) {
          Serial.println("Handling input for Breakout");
          if (currentInput == MENU) {
            currentApp = APP_MENU;
            breakout.exitBreakout();
          }
        }
        else if (currentApp == APP_TETRIS) {
          Serial.println("Handling input for Tetris");
          if (currentInput == MENU) {
            currentApp = APP_MENU;
            tetris.exitTetris();
          }
        }
    }
}