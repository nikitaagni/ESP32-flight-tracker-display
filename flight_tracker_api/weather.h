#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h" 
#include "config.h"

const char* API_URL_FORMAT = "https://api.openweathermap.org/data/3.0/onecall?lat=%.4f&lon=%.4f&exclude=current,minutely,hourly,alerts&appid=%s&units=imperial";

extern const uint8_t sun24x24[24][3] PROGMEM;
extern const uint8_t cloud24x24[24][3] PROGMEM;
extern const uint8_t rain24x24[24][3] PROGMEM;
extern const uint8_t snow24x24[24][3] PROGMEM;

// --- Weather Data Structure ---
struct WeatherData {
    int temp_high = 0;
    int temp_low = 0;
    String day_abbr = "---";
    String condition = "Clear"; 
};

// Global Variable Declarations
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

String getDayAbbr(time_t epoch_time);
void getWeather();
void updateDisplay();
void drawIcon(int x, int y, const uint8_t icon[24][3], uint16_t color);

const uint8_t (*getWeatherIcon(const String& condition))[3];

#endif // WEATHER_H
