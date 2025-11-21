#include "weather.h"

// --- Global Variable Definitions (Matching extern declarations in weather.h) ---
WeatherData weatherForecast;
unsigned long lastWeatherUpdate = 0;
const long WEATHER_UPDATE_INTERVAL = 30 * 60 * 1000; // Update weather every 30 minutes

// --- ICON BITMAPS (12x12 pixels) ---
const unsigned char PROGMEM sun_icon[] = {
  0x00, 0x00, 0x18, 0x24, 0x42, 0x81, 0x81, 0x42, 0x24, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x18, 0x24, 0x42, 0x81, 0x81, 0x42, 0x24, 0x18, 0x00, 0x00
};

const unsigned char PROGMEM rain_icon[] = {
  0x00, 0x00, 0x00, 0x3C, 0x7E, 0xFE, 0xFE, 0x7E, 0x3C, 0x42, 0x42, 0x84,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


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
    const unsigned char* icon = sun_icon; // Default to sun

    condition.toLowerCase();

    if (condition.indexOf("rain") != -1 || condition.indexOf("drizzle") != -1) {
        icon = rain_icon;
        color = COLOR_BLUE;
    } else if (condition.indexOf("cloud") != -1) {
        icon = rain_icon; 
        color = COLOR_CYAN;
    } else { // Clear or Sun
        icon = sun_icon;
        color = COLOR_YELLOW;
    }

    drawIcon(x, y, color, icon, 12, 12); 
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

        // --- Extract Day 1 (Tomorrow) Data ---
        weatherForecast.temp_high_day1 = doc["daily"][1]["temp"]["max"].as<int>();
        weatherForecast.temp_low_day1 = doc["daily"][1]["temp"]["min"].as<int>();
        time_t dt1 = doc["daily"][1]["dt"].as<time_t>();
        weatherForecast.day_abbr_day1 = getDayAbbr(dt1);
        
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
    dma_display->setCursor(5, PANEL_RES_Y - 9); 
    dma_display->print(weatherForecast.day_abbr_day0); 

    // --- RIGHT HALF: TIME & DAY 1 FORECAST ---

    // 1. Draw Day 1 Abbreviation (Top Right Corner - e.g., "SUN")
    dma_display->setTextSize(1);
    dma_display->setTextColor(COLOR_WHITE);
    // Start X position to leave space for the temps
    dma_display->setCursor(PANEL_RES_X - 25, 5); 
    dma_display->print(weatherForecast.day_abbr_day1); 
    
    // 2. Draw the High/Low Temps for Day 1
    dma_display->setTextSize(1);
    dma_display->setTextColor(COLOR_WHITE);
    
    // --- ROW 1: High Temp (Right Side) ---
    dma_display->setCursor(PANEL_RES_X - 24, 15); 
    dma_display->printf("%d%c", weatherForecast.temp_high_day1, DEGREE_CHAR); 
    
    // --- ROW 2: Low Temp (Right Side) ---
    dma_display->setCursor(PANEL_RES_X - 24, 25); 
    dma_display->printf("%d%c", weatherForecast.temp_low_day1, DEGREE_CHAR); 

    // 3. Draw the requested time format [8: \newline 50]
    dma_display->setTextSize(1); // Large font
    dma_display->setTextColor(COLOR_GREEN);

    // X position centered around the 32-pixel mark
    const int TIME_X_START = 34; 

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


/**
 * @brief Handles initialization of display, WiFi, and NTP.
 */
void initializeSystem(const char* ssid, const char* password, long gmtOffset, int daylightOffset, const char* ntpServer) {
    // 1. Initialize Display
    dma_display->begin();
    dma_display->setBrightness8(60);
    dma_display->setTextWrap(false);
    dma_display->fillScreen(COLOR_BLACK);
    dma_display->flipDMABuffer();
    
    // 2. Connect to WiFi
    dma_display->setCursor(0, 0);
    dma_display->print("Connecting...");
    dma_display->flipDMABuffer();

    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected.");
        // 3. Initialize NTP Time
        configTime(gmtOffset, daylightOffset, ntpServer);
    } else {
        Serial.println("\nWiFi connection failed.");
        dma_display->fillScreen(COLOR_BLACK);
        dma_display->setCursor(0, 0);
        dma_display->setTextColor(dma_display->color565(255, 0, 0));
        dma_display->print("WIFI ERR");
        dma_display->flipDMABuffer();
        while(1) delay(1000); // Stop here if no WiFi
    }
}
