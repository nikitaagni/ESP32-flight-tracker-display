#ifndef FLIGHT_H
#define FLIGHT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

const char* opensky_url = "https://opensky-network.org/api/states/all";
const char* aeroBase = "https://aeroapi.flightaware.com/aeroapi/flights/";
const char* aeroAPIKey = AEROAPIKEY;

// Structure for a local flight object
struct Flight {
    String callsign;
    float distance;
};


// Main function to fetch flight data
void getNearestFlightInfo();

// Helper to fetch details for a specific flight
bool fetchFlightAwareData(String callsign);

// Simple Haversine distance calculation
float haversine(float lat1, float lon1, float lat2, float lon2);

void displayFlightInfo(String origin, String dest, String airline, String flight, String aircraft);


#endif // FLIGHT_H
