#include "weather.h"

// --- Global Variable Definitions (Matching extern declarations in weather.h) ---
WeatherData weatherForecast;
unsigned long lastWeatherUpdate = 0;
const long WEATHER_UPDATE_INTERVAL = 30 * 60 * 1000; // Update weather every 30 minutes


// New Sun icon with central circle and 8 rays for better visual appeal
// 24x24 SUN BITMAP (1 = pixel on)
// const uint8_t sun24x24[24][3] PROGMEM = {
//  { 0b11111111, 0b11100000, 0b00000000 },
//  { 0b11111111, 0b11111000, 0b00000000 },
//  { 0b11111111, 0b11111100, 0b00000000 },
//  { 0b11111111, 0b11111110, 0b00000000 },
//  { 0b11111111, 0b11111110, 0b00000000 },
//  { 0b11111111, 0b11111111, 0b00000000 },
//  { 0b11111111, 0b11111111, 0b00000000 },
//  { 0b11111111, 0b11111111, 0b00000000 },
//  { 0b11111111, 0b11111111, 0b00000000 },
//  { 0b11111111, 0b11111111, 0b00000000 },
//  { 0b11111111, 0b11111111, 0b00000000 },
//  { 0b11111111, 0b11111111, 0b00000000 },
//  { 0b11111111, 0b11111111, 0b00000000 },
//  { 0b11111111, 0b11111110, 0b00000000 },
//  { 0b11111111, 0b11111110, 0b00000000 },
//  { 0b11111111, 0b11111100, 0b00000000 },
//  { 0b01111111, 0b11111000, 0b00000000 },
//  { 0b00011111, 0b11100000, 0b00000000 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 }
//};
//
//const uint8_t cloud24x24[24][3] PROGMEM = {
//  {0b00000000,0b01111000,0b00000000},
//  {0b00000001,0b11111100,0b00000000},
//  {0b00001111,0b11111110,0b00000000},
//  {0b00001111,0b11111111,0b00000000},
//  {0b00111111,0b11111111,0b00000000},
//  {0b00111111,0b11111111,0b11000000},
//  {0b01111111,0b11111111,0b11100000},
//  {0b01111111,0b11111111,0b11110000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b01111111,0b11111111,0b11110000},
//  {0b00000000,0b00000000,0b00000000},
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 },
//  { 0,0,0 }
//};
//
//const uint8_t rain24x24[24][3] PROGMEM = {
//    {0b00000000,0b01111000,0b00000000},
//  {0b00000001,0b11111100,0b00000000},
//  {0b00001111,0b11111110,0b00000000},
//  {0b00001111,0b11111111,0b00000000},
//  {0b00111111,0b11111111,0b00000000},
//  {0b00111111,0b11111111,0b11000000},
//  {0b01111111,0b11111111,0b11100000},
//  {0b01111111,0b11111111,0b11110000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b01111111,0b11111111,0b11110000},
//
//  {0b00110000,0b00110000,0b00110000}, // raindrops start
//  {0b00010000,0b00100000,0b00100000},
//  {0b00110000,0b00110000,0b00110000},
//  {0b00000000,0b00000000,0b00000000},
//  {0b00010000,0b00100000,0b00100000},
//  {0b00110000,0b00110000,0b00110000},
//  {0b00000000,0b00000000,0b00000000},
//  {0b00010000,0b00100000,0b00100000},
//  {0b00110000,0b00110000,0b00110000},
//  {0b00000000,0b00000000,0b00000000},
//  {0b00000000,0b00000000,0b00000000}
//};
//
//const uint8_t snow24x24[24][3] PROGMEM = {
//  {0b00000000,0b01111000,0b00000000},
//  {0b00000001,0b11111100,0b00000000},
//  {0b00001111,0b11111110,0b00000000},
//  {0b00001111,0b11111111,0b00000000},
//  {0b00111111,0b11111111,0b00000000},
//  {0b00111111,0b11111111,0b11000000},
//  {0b01111111,0b11111111,0b11100000},
//  {0b01111111,0b11111111,0b11110000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b11111111,0b11111111,0b11111000},
//  {0b01111111,0b11111111,0b11110000},
//
//  // snowflakes
//  {0b00100000,0b00000000,0b00100000},
//  {0b10101000,0b00000000,0b10101000},
//  {0b01110000,0b00000000,0b01110000},
//  {0b11111000,0b00000000,0b11111000},
//  {0b01110000,0b00010000,0b01110000},
//  {0b10101000,0b01010100,0b10101000},
//  {0b00100000,0b00111000,0b00100000},
//  {0b00000000,0b01111100,0b00000000},
//  {0b00000000,0b00111000,0b00000000},
//  {0b00000000,0b01010100,0b00000000},
//  {0b00000000,0b00010000,0b00000000}
//};


/**
 * @brief Draws a simple 12x12 icon.
 */
void drawIcon(int x, int y, uint16_t color, const unsigned char* icon, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (icon[i] & (1 << (width - 1 - j))) {
                dma_display->drawPixel(x + j, y + i, color);
            }
        }
    }
}

/**
 * @brief Selects icon and color based on weather condition and draws it.
 */
void drawWeatherIcon(int x, int y, String condition) {
    uint16_t color = COLOR_WHITE;
//    const unsigned char* icon = sun_icon; // Default to sun
//
//    condition.toLowerCase();
//
//    if (condition.indexOf("rain") != -1 || condition.indexOf("drizzle") != -1) {
//        icon = rain_icon;
//        color = COLOR_BLUE;
//    } else if (condition.indexOf("cloud") != -1) {
//        icon = rain_icon; 
//        color = COLOR_CYAN;
//    } else { // Clear or Sun
//        icon = sun_icon;
//        color = COLOR_YELLOW;
//    }
//
//    drawIcon(x, y, color, icon, 12, 12); 
}

/**
 * @brief Converts epoch time to a 3-letter day abbreviation.
 */
String getDayAbbr(time_t epoch_time) {
    struct tm *lt = localtime(&epoch_time); 
    char day_abbr[4];
    strftime(day_abbr, sizeof(day_abbr), "%a", lt); 
    String day =  String(day_abbr);
    day.toUpperCase();
    return day;
}


/**
 * @brief Fetches weather data from OpenWeatherMap One Call 3.0.
 */
void getWeather() {
    Serial.print("Fetching weather data...");
    
    char url[300];
    // Use the #define variables here: LAT, LON, and WEATHERAPIKEY
    snprintf(url, sizeof(url), API_URL_FORMAT, (double)LAT, (double)LON, EXCLUDE_PARTS, WEATHERAPIKEY);

    HTTPClient http;
    // Note: The OpenWeatherMap API uses HTTPS, which is handled automatically by ESP32 HTTPClient.
    http.begin(url); 
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();
        // Use a small buffer size (512) as we only need two days of forecast
        StaticJsonDocument<512> doc; 
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            http.end();
            return;
        }

        // --- Extract Day 0 (Today) Data ---
        weatherForecast.temp_high_day0 = doc["daily"][0]["temp"]["max"].as<int>();
        weatherForecast.temp_low_day0 = doc["daily"][0]["temp"]["min"].as<int>();
        weatherForecast.condition_day0 = doc["daily"][0]["weather"][0]["main"].as<String>();
        time_t dt0 = doc["daily"][0]["dt"].as<time_t>();
        weatherForecast.day_abbr_day0 = getDayAbbr(dt0);

        
        Serial.println("Success!");
    } else {
        Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    lastWeatherUpdate = millis();
}

/**
 * @brief Renders the entire display content (Clock, Weather Icon, Temps).
 */
void updateDisplay() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return; 
    }

    // Clear the screen
    dma_display->fillScreen(COLOR_BLACK);
    
    // --- LAYOUT CONSTANTS (Based on 64x32) ---
    const int DEGREE_CHAR = 248; 
    
    // --- LEFT HALF: WEATHER ICON & DAY 0 ---
    
    // 1. Draw the large weather icon (12x12, centered on the left)
    drawWeatherIcon(10, 5, weatherForecast.condition_day0); 

    // 2. Draw Day 0 Abbreviation (e.g., "SAT")
    dma_display->setTextSize(1); 
    dma_display->setTextColor(COLOR_WHITE);
    // Positioned at the bottom left
    dma_display->setCursor(5, 23); 
//    dma_display->print(weatherForecast.day_abbr_day0); 
dma_display->print("SAT"); 

    // --- RIGHT HALF: TIME & DAY 1 FORECAST ---

//    // 1. Draw Day 1 Abbreviation (Top Right Corner - e.g., "SUN")
//    dma_display->setTextSize(1);
//    dma_display->setTextColor(COLOR_WHITE);
//    // Start X position to leave space for the temps
//    dma_display->setCursor(PANEL_RES_X - 25, 5); 
//    dma_display->print(weatherForecast.day_abbr_day0); 
    
    // 2. Draw the High/Low Temps for Day 1
    dma_display->setTextSize(1);
    dma_display->setTextColor(COLOR_WHITE);
    
    // --- ROW 1: High Temp (Right Side) ---
    dma_display->setCursor(35, 12); 
    dma_display->printf("%d%c", 70, DEGREE_CHAR); 
//    dma_display->printf("%d%c", weatherForecast.temp_high_day0, DEGREE_CHAR); 
    
    // --- ROW 2: Low Temp (Right Side) ---
    dma_display->setCursor(PANEL_RES_X - 24, 23); 
    dma_display->printf("%d%c", 65, DEGREE_CHAR); 
//    dma_display->printf("%d%c", weatherForecast.temp_low_day0, DEGREE_CHAR); 

    // 3. Draw the requested time format [8: \newline 50]
    dma_display->setTextSize(1); // Large font
    dma_display->setTextColor(COLOR_BLUE);

    dma_display->drawFastVLine(44, 2, 28, COLOR_WHITE);

    // X position centered around the 32-pixel mark
    const int TIME_X_START = 47; 

    // HOUR: e.g., "8:" (using 12-hour format)
    int hour_12 = timeinfo.tm_hour % 12;
    if (hour_12 == 0) hour_12 = 12; // Handle 0:00 as 12:00
    dma_display->setCursor(TIME_X_START, 5); 
    dma_display->printf("%d:", hour_12); 

    // MINUTES: e.g., "50" (placed on the next line)
    dma_display->setCursor(TIME_X_START, 18); 
    dma_display->printf("%02d", timeinfo.tm_min); // %02d ensures leading zero

    // Final flip to push the buffer to the display
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
