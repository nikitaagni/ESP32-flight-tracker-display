#include "flight.h"
#include <vector>
#include <algorithm>
#include <cmath> // For isnan()

unsigned long lastFlightUpdate = 0;
const long FLIGHT_UPDATE_INTERVAL = 60 * 1000 * 2; // every two minute

// haversine distance (km) calculation between two points.
float haversine(float lat1, float lon1, float lat2, float lon2) {
    float R = 6371; // Earth radius in km
    float dLat = radians(lat2 - lat1);
    float dLon = radians(lon2 - lon1);
    float a = sin(dLat / 2) * sin(dLat / 2) +
              cos(radians(lat1)) * cos(radians(lat2)) *
              sin(dLon / 2) * sin(dLon / 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

// Fetches detailed flight information from a secondary API (like FlightAware/AeroDataBox).
bool fetchFlightAwareData(String callsign) {
    callsign.trim();
    if (callsign.length() < 3) {
        Serial.println("Invalid callsign length.");
        return false;
    }

    HTTPClient http;
    
struct tm timeinfo;
getLocalTime(&timeinfo);

    char startDay[11];
    strftime(startDay, sizeof(startDay), "%Y-%m-%d", &timeinfo);
    
    // Compute tomorrow
    timeinfo.tm_mday += 1;
    mktime(&timeinfo);
    
    char endDay[11];
    strftime(endDay, sizeof(endDay), "%Y-%m-%d", &timeinfo);

    // Construct the URL using the base and callsign
    String url = String(aeroBase) + callsign;
    url += "?ident_type=designator&start=";
    url += startDay;
    url += "&end=";
    url += endDay;


    Serial.println(url);
    http.begin(url);
    
    // Add required headers for the secondary API
    http.addHeader("x-apikey", aeroAPIKey);
    http.addHeader("Accept", "application/json");

    int code = http.GET();
    Serial.printf("FlightAware API response code: %d\n", code);
    
    if (code != 200) {
        Serial.println("FlightAware API failed or returned non-200 code.");
        http.end();
        return false;
    }

    // Use a large buffer since FlightAware data is often detailed
    DynamicJsonDocument doc(4000);
    deserializeJson(doc, http.getString());
    http.end();

    JsonArray flights = doc["flights"];
    if (flights.size() == 0) {
        Serial.printf("No detailed data for callsign: %s\n", callsign.c_str());
        return false;
    }

    JsonObject first = flights[0];

    // Filter out flights that aren't mainline airline traffic
    if (first["type"].as<String>() != "Airline") {
        return false;
    }
//    
//    // Extract data fields
    String origin  = first["origin"]["code_iata"].as<String>();    // e.g. "ATL"
    String dest    = first["destination"]["code_iata"].as<String>(); // e.g. "LAX"
    String flight  = first["ident_iata"].as<String>();            // e.g. "DL640"
    String aircraft = first["aircraft_type"].as<String>();          // e.g. "A321"
//    int knots = flight["filed_airspeed"] | 0;
//    int altitudeHundreds = flight["filed_altitude"] | 0;
//
//    // test data
//    String origin = "ATL";
//    String dest = "SFO";
//    int altitudeHundreds = 500;
//    String flight = "F91949";
//    int knots = 460.312;
//
    Serial.println("Found valid flight!");
    Serial.printf("Route: %s -> %s\n", origin.c_str(), dest.c_str());
//
//    // Call the display function
    displayFlightInfo(origin, dest, flight, aircraft);
//
    return true;
}

/**
 * @brief Fetches state vectors from OpenSky and finds the nearest valid flight.
 */
void getNearestFlightInfo() {

  
    Serial.print("Fetching OpenSky data...");
    
    HTTPClient http;

    // Construct the URL to define a box around the current location (LAT/LON)
    String url = String(opensky_url) + "?lamin=" + String(LAT - 0.5) + "&lomin=" + String(LON - 0.5) +
                 "&lamax=" + String(LAT + 0.5) + "&lomax=" + String(LON + 0.5);

    Serial.println(url);
    http.begin(url);
    int code = http.GET();

    Serial.printf("OpenSky response code: %d\n", code);

    if (code != 200) {
        Serial.println("Failed to get OpenSky data");
        getNearestFlightInfo();
        return;
    }

    String payload = http.getString();
    // OpenSky returns a large JSON, so we use a larger buffer size
    DynamicJsonDocument doc(20000); 
    DeserializationError error = deserializeJson(doc, payload);
    http.end();

    if (error) {
        Serial.print("OpenSky JSON parse failed: ");
        Serial.println(error.f_str());
        return;
    }

    JsonArray states = doc["states"];

    if (!states || states.size() == 0) {
        Serial.println("No planes nearby!");
        return;
    }

    std::vector<Flight> flights;

    // 1. Filter and calculate distance
    for (JsonArray state : states) {
        String callsign = state[1].as<String>();
        bool groundState = state[8];
        float lat = state[6];
        float lon = state[5];
        
        // Basic filtering: requires callsign, valid coords, not on ground, and not private (e.g., 'N' prefix)
        if (callsign.length() < 3 || isnan(lat) || isnan(lon) || groundState || callsign[0] == 'N') continue;

        float dist = haversine(LAT, LON, lat, lon);
        if (dist < maxRadiusKm) {
            flights.push_back({callsign, dist});
        }
    }

    // Sort by distance (nearest first)
    std::sort(flights.begin(), flights.end(), [](Flight a, Flight b) {
        return a.distance < b.distance;
    });
        
    // Secondary API lookup for nearest flights until a valid one is found
    for (auto &f : flights) {
        Serial.printf("Checking callsign: %s, Distance: %.1f km\n", f.callsign.c_str(), f.distance);
        if (fetchFlightAwareData(f.callsign) ) {
            // Found a valid flight with full details, so we stop and display it
            lastFlightUpdate = millis();
            return;
        }
    }

    Serial.println("No valid FlightAware data found for any nearby planes.");
}

void displayFlightInfo(String origin, String dest, String flight, String aircraft) {

    dma_display->clearScreen();

    // ORIGIN → DEST
    dma_display->setTextColor(dma_display->color565(255, 153, 255)); // pink
    dma_display->setFont(NULL);
    dma_display->setTextSize(1);
    dma_display->setCursor(5, 3);
    dma_display->print(origin);
    dma_display->print(" > ");
    dma_display->print(dest);

    // FLIGHT NUMBER
    dma_display->setTextColor(dma_display->color565(0, 128, 255)); // blue
    dma_display->setTextSize(1);  
    dma_display->setCursor(5, 13);
    dma_display->print(flight);

    
    dma_display->drawFastHLine(43, 16, 16, dma_display->color565(0, 128, 255));

    dma_display->setTextColor(dma_display->color565(0, 153, 52)); // purple
    dma_display->setTextSize(1);
    dma_display->setCursor(5, 23);
    dma_display->print(aircraft);


//    int mph = knots * 1.15078;              // speed in mph
//    int feet = altHundreds * 100;
//    int miles = feet / 5280.0;
//
//    // AIRCRAFT
//    dma_display->setTextColor(dma_display->color565(0, 153, 52)); // purple
//    dma_display->setTextSize(1);
//    dma_display->setCursor(5, 23);
//
//    dma_display->printf("%d", mph);
//
//    // Draw "mph" in tiny font right after the number
//    dma_display->setFont(&TomThumb);
//    dma_display->setCursor(dma_display->getCursorX(), 30);
//    dma_display->print("mph");
//    
//    // Altitude
//    dma_display->setFont(NULL); // back to large font
//    dma_display->setCursor(dma_display->getCursorX() + 5, 23);
//    dma_display->printf("%d", miles);
//    
//    // Draw "m" in tiny font
//    dma_display->setFont(&TomThumb);
//    dma_display->setCursor(dma_display->getCursorX(), 30);
//    dma_display->print("mi");
//    dma_display->setFont(NULL);

    dma_display->flipDMABuffer();
}
