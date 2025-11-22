#include "weather.h"

// Global Variable Definitions
WeatherData weatherForecast;
unsigned long lastWeatherUpdate = 0;
const long WEATHER_UPDATE_INTERVAL = 30 * 60 * 1000; // Update weather every 30 minutes

// 24x24 SUN BITMAP (1 = pixel on)
 const uint8_t sun24x24[24][3] PROGMEM = {
  { 0b11111111, 0b11100000, 0b00000000 },
  { 0b11111111, 0b11111000, 0b00000000 },
  { 0b11111111, 0b11111100, 0b00000000 },
  { 0b11111111, 0b11111110, 0b00000000 },
  { 0b11111111, 0b11111110, 0b00000000 },
  { 0b11111111, 0b11111111, 0b00000000 },
  { 0b11111111, 0b11111111, 0b00000000 },
  { 0b11111111, 0b11111111, 0b00000000 },
  { 0b11111111, 0b11111111, 0b00000000 },
  { 0b11111111, 0b11111111, 0b00000000 },
  { 0b11111111, 0b11111111, 0b00000000 },
  { 0b11111111, 0b11111111, 0b00000000 },
  { 0b11111111, 0b11111111, 0b00000000 },
  { 0b11111111, 0b11111110, 0b00000000 },
  { 0b11111111, 0b11111110, 0b00000000 },
  { 0b11111111, 0b11111100, 0b00000000 },
  { 0b01111111, 0b11111000, 0b00000000 },
  { 0b00011111, 0b11100000, 0b00000000 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 }
};

const uint8_t cloud24x24[24][3] PROGMEM = {
  {0b00000000,0b01111000,0b00000000},
  {0b00000001,0b11111100,0b00000000},
  {0b00001111,0b11111110,0b00000000},
  {0b00001111,0b11111111,0b00000000},
  {0b00111111,0b11111111,0b00000000},
  {0b00111111,0b11111111,0b11000000},
  {0b01111111,0b11111111,0b11100000},
  {0b01111111,0b11111111,0b11110000},
  {0b11111111,0b11111111,0b11111000},
  {0b11111111,0b11111111,0b11111000},
  {0b11111111,0b11111111,0b11111000},
  {0b11111111,0b11111111,0b11111000},
  {0b01111111,0b11111111,0b11110000},
  {0b00000000,0b00000000,0b00000000},
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 }
};

const uint8_t rain24x24[24][3] PROGMEM = {
    {0b00000000,0b01111000,0b00000000},
  {0b00000001,0b11111100,0b00000000},
  {0b00001111,0b11111110,0b00000000},
  {0b00001111,0b11111111,0b00000000},
  {0b00111111,0b11111111,0b00000000},
  {0b00111111,0b11111111,0b11000000},
  {0b01111111,0b11111111,0b11100000},
  {0b01111111,0b11111111,0b11110000},
  {0b11111111,0b11111111,0b11111000},
  {0b11111111,0b11111111,0b11111000},
  {0b11111111,0b11111111,0b11111000},
  {0b01111111,0b11111111,0b11110000},
  { 0,0,0 },

  {0b00010000,0b00100000,0b00100000}, // raindrops start
  {0b00010000,0b00100000,0b00100000},
  {0b00010000,0b00100000,0b00100000},
  {0b00000001,0b00000010,0b00000000},
  {0b00000001,0b00000010,0b00000000},
  {0b00000001,0b00000010,0b00000000},
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 }
};

const uint8_t snow24x24[24][3] PROGMEM = {
  {0b00000000,0b01111000,0b00000000},
  {0b00000001,0b11111100,0b00000000},
  {0b00000111,0b11111110,0b00000000},
  {0b00001111,0b11111111,0b00000000},
  {0b00011111,0b11111111,0b00000000},
  {0b00111111,0b11111111,0b11000000},
  {0b01111111,0b11111111,0b11100000},
  {0b11111111,0b11111111,0b11110000},
  {0b11111111,0b11111111,0b11111000},
  {0b11111111,0b11111111,0b11111000},
  {0b01111111,0b11111111,0b11110000},
  { 0,0,0 },

  // snowflakes
  {0b00100000,0b00000000,0b00010000},
  {0b10101000,0b00000000,0b01010100},
  {0b01110000,0b00100000,0b00111000},
  {0b11111000,0b10101000,0b01111100},
  {0b01110000,0b01110000,0b00111000},
  {0b10101000,0b11111000,0b01010100},
  {0b00100000,0b01110000,0b00010000},
  {0b00000000,0b10101000,0b00000000},
  {0b00000000,0b00100000,0b00000000},
  {0b00000000,0b00000000,0b00000000},
  {0b00000000,0b00000000,0b00000000}
};

// Converts epoch time to a 3-letter day abbreviation.
String getDayAbbr(time_t epoch_time) {
    struct tm *lt = localtime(&epoch_time); 
    char day_abbr[4];
    strftime(day_abbr, sizeof(day_abbr), "%a", lt); 
    String day =  String(day_abbr);
    day.toUpperCase();
    return day;
}


// Fetches weather data from OpenWeatherMap One Call 3.0.
void getWeather() {
    Serial.print("Fetching weather data...");
    
    char url[300];
    // Use the #define variables here: LAT, LON, and WEATHERAPIKEY
    snprintf(url, sizeof(url), API_URL_FORMAT, (double)LAT, (double)LON, WEATHERAPIKEY);

    HTTPClient http;
    // Note: The OpenWeatherMap API uses HTTPS, which is handled automatically by ESP32 HTTPClient.
    http.begin(url); 

    Serial.print(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();
        // Use a small buffer size (512) as we only need two days of forecast
        StaticJsonDocument<512> doc; 
        DeserializationError error = deserializeJson(doc, payload);
        
        // --- ADDED: Serial Print of Raw Payload ---
        Serial.println("Success! Raw JSON Payload:");
        Serial.println(payload);

        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            http.end();
            return;
        }

        // --- Extract Data ---
        weatherForecast.temp_high = doc["daily"][0]["temp"]["max"].as<int>();
        weatherForecast.temp_low = doc["daily"][0]["temp"]["min"].as<int>();
        weatherForecast.condition = doc["daily"][0]["weather"][0]["main"].as<String>();
        time_t dt0 = doc["daily"][0]["dt"].as<time_t>();
        weatherForecast.day_abbr = getDayAbbr(dt0);
        
        Serial.println("Data:");
        Serial.print("  Day Abbreviation: ");
        Serial.println(weatherForecast.day_abbr);
        Serial.print("  High Temperature: ");
        Serial.println(weatherForecast.temp_high);
        Serial.print("  Low Temperature: ");
        Serial.println(weatherForecast.temp_low);
        Serial.print("  Condition: ");
        Serial.println(weatherForecast.condition);

    } else {
        http.end();
        Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
        getWeather();
        return;
    }

    http.end();
    lastWeatherUpdate = millis();
}

// Renders the display content (Clock, Weather Icon, Temps).
void updateDisplay() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return; 
    }

    const int DEGREE_CHAR = 248; 
    
    // --- LEFT HALF: WEATHER ICON---
    pickCondition(weatherForecast.condition);
//    pickCondition("Snow");

    dma_display->setTextSize(1); 
    dma_display->setTextColor(COLOR_WHITE);
    // Positioned at the bottom left
    dma_display->setCursor(3, 23); 
    dma_display->print(weatherForecast.day_abbr); 
    
    // 2. Draw the High/Low Temps for Day 1
    dma_display->setTextSize(1);
    dma_display->setTextColor(COLOR_WHITE);

    const int tempX = 29;
    
    // --- ROW 1: High Temp (Right Side) ---
    dma_display->setCursor(tempX, 12); 
    dma_display->printf("%d", weatherForecast.temp_high); 
    dma_display->setCursor(tempX+10, 9); 
    dma_display->printf("%c", DEGREE_CHAR); 
    
    // --- ROW 2: Low Temp (Right Side) ---
    dma_display->setCursor(tempX, 23); 
    dma_display->printf("%d", weatherForecast.temp_low);
    dma_display->setCursor(tempX+10, 20); 
    dma_display->printf("%c", DEGREE_CHAR);  

    // TIME
    dma_display->setTextSize(1);
    dma_display->setTextColor(COLOR_BLUE);

    dma_display->drawFastVLine(46, 2, 26, COLOR_WHITE);

    // X position 
    const int TIME_X_START = 50; 
    dma_display->setFont(&FreeSerif9pt7b);
    // HOUR: e.g., "8:" (using 12-hour format)
    int hour_12 = timeinfo.tm_hour % 12;
    if (hour_12 == 0) hour_12 = 12; // Handle 0:00 as 12:00
    dma_display->setCursor(TIME_X_START, 8); 
    dma_display->printf("%d", hour_12); 

    // MINUTES: e.g., "50" (placed on the next line)
    dma_display->setCursor(TIME_X_START, 18); 
    dma_display->printf("%02d", timeinfo.tm_min); // %02d ensures leading zero
    dma_display->setFont(NULL);

    dma_display->flipDMABuffer();
}

void drawIcon(int x, int y, const uint8_t icon[24][3], uint16_t color) {
    for (int row = 0; row < 24; row++) {
        for (int byteCol = 0; byteCol < 3; byteCol++) {
            uint8_t b = pgm_read_byte(&(icon[row][byteCol])); // <--- read from PROGMEM
            for (int bit = 7; bit >= 0; bit--) {
                if (b & (1 << bit)) {
                    int col = byteCol*8 + (7-bit);
                    dma_display->drawPixel(x + col, y + row, color);
                }
            }
        }
    }
}

void pickCondition(const String& condition) {
    if (condition == "Thunderstorm" ||
        condition == "Drizzle" ||
        condition == "Rain") 
    {
       drawIcon(1, 1, rain24x24, dma_display->color565(0, 120, 255));
        return;
    }

    if (condition == "Snow") {
        drawIcon(1, 1, snow24x24, dma_display->color565(180, 240, 255));
        return;
    }

    // “Atmosphere” includes Mist, Smoke, Haze, Fog, Sand, Dust, Ash, Squall, Tornado
    if (condition == "Atmosphere" ||
        condition == "Mist" ||
        condition == "Smoke" ||
        condition == "Haze" ||
        condition == "Dust" ||
        condition == "Fog" ||
        condition == "Sand" ||
        condition == "Ash" ||
        condition == "Squall" ||
        condition == "Tornado" ||
        condition == "Clouds") 
    {
       drawIcon(2, 2, cloud24x24, dma_display->color565(180, 200, 220));
       drawIcon(10, 10, cloud24x24, dma_display->color565(102, 102, 102));
        return;
    }

    // Default fallback
    drawIcon(0, 0, sun24x24, dma_display->color565(255, 220, 0));
    return;
}
