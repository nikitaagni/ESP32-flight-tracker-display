#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h" 
#include "config.h"

// We exclude current, minutely, hourly, and alerts data to keep the JSON small
const char* EXCLUDE_PARTS = "current,minutely,hourly,alerts";
const char* API_URL_FORMAT = "https://api.openweathermap.org/data/3.0/onecall?lat=%.4f&lon=%.4f&exclude=%s&units=metric&appid=%s";


// --- Weather Data Structure ---
struct WeatherData {
    // Day 0 (Today)
    int temp_high_day0 = 0;
    int temp_low_day0 = 0;
    String day_abbr_day0 = "---";
    String condition_day0 = "Clear"; 

    // Day 1 (Tomorrow)
    int temp_high_day1 = 0;
    int temp_low_day1 = 0;
    String day_abbr_day1 = "---";
};

// Global Variable Declarations (using extern keyword)
// The definitions (where memory is allocated) are in weather.cpp
extern WeatherData weatherForecast;
extern unsigned long lastWeatherUpdate;
extern const long WEATHER_UPDATE_INTERVAL;

// --- Color Definitions ---
// These must be defined after dma_display object is declared
#define COLOR_WHITE dma_display->color565(255, 255, 255)
#define COLOR_YELLOW dma_display->color565(255, 255, 0)
#define COLOR_CYAN dma_display->color565(0, 255, 255)
#define COLOR_BLUE dma_display->color565(0, 0, 255)
#define COLOR_GREEN dma_display->color565(0, 255, 0)
#define COLOR_BLACK dma_display->color565(0, 0, 0)

// --- Function Prototypes ---
void initializeSystem(const char* ssid, const char* password, long gmtOffset, int daylightOffset, const char* ntpServer);
String getDayAbbr(time_t epoch_time);
void getWeather();
void updateDisplay();

#endif // WEATHER_H
