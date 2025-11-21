#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include "airlines.h"
#include "config.h"
#include "weather.h"
#include <time.h>

#define TIMEZONE "EST5EDT" // change to your timezone


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3600 * 5;
const int   daylightOffset_sec = 3600;


#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

MatrixPanel_I2S_DMA *dma_display = nullptr;

uint16_t myBLACK, myWHITE, myRED, myGREEN, myBLUE;

// Wifi Info
const char* ssid = SSID;
const char* password = PASSWORD

// maxRadius
float maxRadiusKm = 100.0; // only search within 50km

void displayClock() {
//    dma_display->fillScreenRGB888(0, 0, 0); // Clear screen
    dma_display->setFont(&FreeSerifBold9pt7b);
    // Get current time from ESP32
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        dma_display->setTextColor(dma_display->color565(255, 0, 0));
        dma_display->setCursor(5, 10);
        dma_display->print("No Time!");
        dma_display->flipDMABuffer();
        return;
    }

    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    int second = timeinfo.tm_sec;

    int hour12 = hour % 12;        // Convert to 12-hour format
    if (hour12 == 0) hour12 = 12; // Handle midnight/noon
    
    bool isPM = hour >= 12;        // true if PM

    // Colors
//    uint16_t hourColor = dma_display->color565(255, 0, 150);   // orange
    uint16_t minColor  = dma_display->color565(0, 200, 255);   // cyan
//    uint16_t secColor  = dma_display->color565(255, 0, 150);   // pink

    // ==== HOURS ====
    dma_display->setTextSize(1);
    dma_display->setCursor(3, 15);
    dma_display->setTextColor(minColor);
    if (hour12 < 10) dma_display->setCursor(10, 15);;
    dma_display->print(hour12);

    dma_display->setTextColor(minColor);
    dma_display->print(":");

    // ==== MINUTES ====
//    dma_display->setTextColor(minColor);
    if (minute < 10) dma_display->print("0");
    dma_display->print(minute);

    // Cute little animation: moving heart
//    int heartX = (minute * PANEL_RES_X) / 60;
//    int heartY = 28;
//    dma_display->fillRect(heartX, heartY, 4, 4, dma_display->color565(255, 0, 0));
//    dma_display->fillScreenRGB888(0, 0, 0);
    dma_display->flipDMABuffer();
}

void setup() {

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
  dma_display->setBrightness8(60); //0-255
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
}

void loop() {
  delay(6000); // update once per minute
  dma_display->fillScreenRGB888(0, 0, 0);

    // Check if it's time to update the weather data
  if (millis() - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL) {
      getWeather();
  }

  // Always update the display for real-time clock changes
//  updateDisplay();
 displayClock();
//  getNearestFlightInfo();

}
