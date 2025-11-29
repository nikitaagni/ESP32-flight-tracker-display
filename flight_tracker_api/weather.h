#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h" 
#include "config.h"

const char* API_URL_FORMAT = "https://api.openweathermap.org/data/3.0/onecall?lat=%.4f&lon=%.4f&exclude=daily,minutely,hourly,alerts&appid=%s&units=imperial";

extern const uint8_t sun24x24[24][3] PROGMEM;
extern const uint8_t cloud24x24[24][3] PROGMEM;
extern const uint8_t rain24x24[24][3] PROGMEM;
extern const uint8_t snow24x24[24][3] PROGMEM;

// --- Weather Data Structure ---
struct WeatherData {
    int temp = 0;
    int temp_low = 0;
    String day_abbr = "---";
    String condition = "Clear"; 
};

extern WeatherData weatherForecast;
extern unsigned long lastWeatherUpdate;
extern const long WEATHER_UPDATE_INTERVAL;

String getDayAbbr(time_t epoch_time);
void getWeather();
void updateDisplay();
void drawIcon(int x, int y, const uint8_t icon[24][3], uint16_t color);

const uint8_t (*getWeatherIcon(const String& condition))[3];

#endif // WEATHER_H
