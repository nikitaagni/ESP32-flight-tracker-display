#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "airlines.h"
#include "config.h"

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

MatrixPanel_I2S_DMA *dma_display = nullptr;

uint16_t myBLACK, myWHITE, myRED, myGREEN, myBLUE;

// Wifi Info
const char* ssid = SSID;
const char* password = PASSWORD;

// API Info
const char* opensky_url = "https://opensky-network.org/api/states/all";
const char* aeroBase = "https://aeroapi.flightaware.com/aeroapi/flights/";
const char* aeroAPIKey = AEROAPIKEY;

// maxRadius
float maxRadiusKm = 25.0; // only search within 50km

void displayFlightInfo(String origin, String dest, String airline, String flight, String aircraftOrStatus) {
    dma_display->fillScreenRGB888(0, 0, 0);

    // ==== LINE 1: ORIGIN → DEST ====
    dma_display->setTextColor(dma_display->color565(255, 153, 255)); // pink
    dma_display->setTextSize(1);  // large text
    dma_display->setCursor(5, 3);
    dma_display->print(origin);
    dma_display->print(" > ");
    dma_display->print(dest);

    // ==== LINE 2: FLIGHT NUMBER ====
    dma_display->setTextColor(dma_display->color565(0, 128, 255)); // blue
    dma_display->setTextSize(1);  
    dma_display->setCursor(5, 13);
//    const char* name = airline.c_str();
//    dma_display->print(getAirlineName(name));
//    dma_display->print(getAirlineName(icao_airline.c_str()));
    dma_display->print(flight);
    
    dma_display->drawFastHLine(43, 16, 16, dma_display->color565(0, 128, 255));

    // ==== LINE 3: AIRCRAFT or STATUS ====
    dma_display->setTextColor(dma_display->color565(0, 153, 52)); // purple
    dma_display->setTextSize(1);
    dma_display->setCursor(5, 23);
    dma_display->print(aircraftOrStatus);

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
  getNearestFlightInfo();
}

void loop() {
  delay(6000); // update once per minute
  getNearestFlightInfo();
}

void getNearestFlightInfo() {
  HTTPClient http;

  String url = String(opensky_url) + "?lamin=" + String(LAT - 0.5) + "&lomin=" + String(LON - 0.5) +
               "&lamax=" + String(LAT + 0.5) + "&lomax=" + String(LON + 0.5);

  Serial.println(url);
  http.begin(url);
  int code = http.GET();

  Serial.println(code);

  if (code != 200) {
    Serial.println("Failed to get OpenSky data");
    return;
  }

  String payload = http.getString();
  
  DynamicJsonDocument doc(20000);
  DeserializationError error = deserializeJson(doc, payload);
  http.end();

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.f_str());
    return;
  }

  JsonArray states = doc["states"];


  if (!states || states.size() == 0) {
    Serial.println("No planes nearby!");
    return;
  }

  struct Flight {
    String callsign;
    float distance;
  };
  std::vector<Flight> flights;

  for (JsonArray state : states) {
    String callsign = state[1].as<String>();
    bool groundState = state[8];
    float lat = state[6];
    float lon = state[5];
    if (callsign.length() < 3 || isnan(lat) || isnan(lon) || groundState || callsign[0] == 'N') continue;

    float dist = haversine(LAT, LON, lat, lon);
    if (dist < maxRadiusKm) flights.push_back({callsign, dist});
  }

  std::sort(flights.begin(), flights.end(), [](Flight a, Flight b) {
    return a.distance < b.distance;
  });
   
  for (Flight v : flights) {
    Serial.println(v.callsign);
  }

  // Try FlightAware until valid flight found
  for (auto &f : flights) {
    if (fetchFlightAwareData(f.callsign) ) {
      Serial.println("Found valid flight!");
      return;
    }
  }

  Serial.println("No valid FlightAware data found for any nearby planes.");
}

bool fetchFlightAwareData(String callsign) {
  callsign.trim();
  if (callsign == "") return false;

  HTTPClient http;

  String url = aeroBase + callsign;

  Serial.println(url);
  http.begin(url);
  
//  http.begin();
  http.addHeader("x-apikey", aeroAPIKey);
  http.addHeader("Accept", "application/json");

  int code = http.GET();

   Serial.println(code);
   
  if (code != 200) {
    Serial.println("FlightAware API failed");
    http.end();
    return false;
  }

  DynamicJsonDocument doc(4000);
  deserializeJson(doc, http.getString());
  http.end();

  JsonArray flights = doc["flights"];
  if (flights.size() == 0) {
    Serial.print("No data for ");
    Serial.println(callsign);
    return false;
  }

  // Example output
  JsonObject first = flights[0];

  if (first["type"].as<String>() != "Airline") {
    return false;
  }
  Serial.print("Flight: "); Serial.println(first["ident"].as<String>());
  Serial.print("Origin: "); Serial.println(first["origin"]["name"].as<String>());
  Serial.print("Destination: "); Serial.println(first["destination"]["name"].as<String>());
  Serial.print("Status: "); Serial.println(first["status"].as<String>());

String origin = first["origin"]["code_iata"].as<String>();         // e.g. "ATL"
String dest   = first["destination"]["code_iata"].as<String>();    // e.g. "LAX"
String airline = first["operator_iata"].as<String>();                  // e.g. "DL"
String flight = first["flight_number"].as<String>();                  // e.g. "640"
String aircraft = first["aircraft_type"].as<String>();        // e.g. "A321"

//String origin = "ATL";
//String dest = "SFO";
//String flight = "F91949";
//String aircraft = "A321neo";

// If aircraft is empty, use status
//if (aircraft.length() == 0) {
//    aircraft = first["status"].as<String>();
//}

displayFlightInfo(origin, dest, airline, flight, aircraft);

  return true;
}

// Simple haversine distance (km)
float haversine(float lat1, float lon1, float lat2, float lon2) {
  float R = 6371; // km
  float dLat = radians(lat2 - lat1);
  float dLon = radians(lon2 - lon1);
  float a = sin(dLat / 2) * sin(dLat / 2) +
            cos(radians(lat1)) * cos(radians(lat2)) *
            sin(dLon / 2) * sin(dLon / 2);
  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return R * c;
}
