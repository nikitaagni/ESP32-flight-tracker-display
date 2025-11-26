#include <WiFi.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/TomThumb.h>
#include "config.h"
#include "weather.h"
#include "flight.h"


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3600 * 5;
const int   daylightOffset_sec = 3600;

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

#define LIGHT_SENSOR_PIN 35
#define BUTTON_PIN 33

int currentMode = 0;
unsigned long lastPress = 0;
const unsigned long debounceMs = 1000;
static unsigned long lastUpdate = 0;

MatrixPanel_I2S_DMA *dma_display = nullptr;

// Wifi Info
const char* ssid = SSID;
const char* password = PASSWORD;

// maxRadius
float maxRadiusKm = 100.0; // only search within 50km
int brightness = 80;

void displayClock() {
    dma_display->setFont(&FreeSerif9pt7b);
    // Get current time from ESP32
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }

    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;

    int hour12 = hour % 12;        // Convert to 12-hour format
    if (hour12 == 0) hour12 = 12; // Handle midnight/noon
    
    bool isPM = hour >= 12;        // true if PM

    // Colors
//    uint16_t hourColor = dma_display->color565(255, 0, 150);   // orange
    uint16_t timeColor  = dma_display->color565(0, 200, 255);   // cyan
//    uint16_t secColor  = dma_display->color565(255, 0, 150);   // pink

    // ==== HOURS ====
    dma_display->setTextSize(1);
    dma_display->setCursor(9, 21);
    dma_display->setTextColor(timeColor);
    if (hour12 < 10) dma_display->setCursor(15, 21);
    dma_display->print(hour12);

//    dma_display->setTextColor(minColor);
    dma_display->print(":");

    // ==== MINUTES ====
//    dma_display->setTextColor(timeColor);
    if (minute < 10) dma_display->print("0");
    dma_display->print(minute);
    dma_display->setFont(NULL);

    dma_display->flipDMABuffer();
}

void handleButton() {
  Serial.println(digitalRead(BUTTON_PIN));
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long now = millis();
    if (now - lastPress > debounceMs) {
      currentMode = (currentMode + 1) % 2;
      lastPress = now;
      dma_display->clearScreen();
      dma_display->setCursor(0, 15);
      dma_display->setFont(&FreeSans9pt7b);
      dma_display->setTextSize(1);
      dma_display->setTextColor(dma_display->color565(255, 255, 255));
      dma_display->print("loading!");
      dma_display->setFont(NULL);
      Serial.println("Getting new data...");

    }
  }
}

void setup() {
    analogSetAttenuation(ADC_11db);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
   HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    PANEL_CHAIN    // Chain length
  );

  mxconfig.gpio.e = 32;
  mxconfig.clkphase = false;
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(brightness); //0-255
  dma_display->clearScreen();
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // display clock while weather loading
  displayClock();
  getWeather();
  dma_display->clearScreen();
  updateDisplay();
  
}

void loop() {
  
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  brightness = map(analogValue, 0, 4095, 16, 255); // Avoid 0 brightness
  dma_display->setBrightness(brightness);

  handleButton();

  if (currentMode == 0) {
     // Check if it's time to update the weather data
    if (millis() - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL) {
      getWeather();
    }
    // otherwise update the time every 3 seconds, needed to prevent display flickering
    if (millis() - lastUpdate > 3000) {
        lastUpdate = millis();
        dma_display->clearScreen();
        updateDisplay();
    }
  } else if (currentMode == 1) {
    // Check if it's time to update the flight data
    if (millis() - lastFlightUpdate > FLIGHT_UPDATE_INTERVAL) {
      getNearestFlightInfo();
    }
  }
}
