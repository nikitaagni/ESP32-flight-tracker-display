#ifndef AIRLINES_H
#define AIRLINES_H

#include <Arduino.h>
#include <pgmspace.h>

struct Airline {
  const char* icao;  // or IATA if your API gives that
  const char* name;
};

// Put the array in PROGMEM so it stays in flash memory
const Airline airlines[] PROGMEM = {
  {"ALK", "Alaska Airlines"},
  {"AAL", "American Airlines"},
  {"DL",  "Delta Air Lines"},
  {"F9",  "Frontier Airlines"},
  {"B6",  "JetBlue Airways"},
  {"WN",  "Southwest Airlines"},
  {"NK",  "Spirit Airlines"},
  {"UA",  "United Airlines"},
  {"AMX", "AeroMexico"},
  {"AC",  "Air Canada"},
  {"AF",  "Air France"},
  {"BA",  "British Airways"},
  {"CM",  "Copa Airlines"},
  {"ET",  "Ethiopian Airlines"},
  {"EY",  "Etihad Airways"},
  {"KL",  "KLM Royal Dutch Airlines"},
  {"KE",  "Korean Air"},
  {"LA",  "LATAM Airlines"},
  {"LH",  "Lufthansa German Airlines"},
  {"QR",  "Qatar Airways"},
  {"SK",  "Scandinavian Airlines"},
  {"TK",  "Turkish Airlines"},
  {"VS",  "Virgin Atlantic"},
  {"WS",  "WestJet"}
};

const int numAirlines = sizeof(airlines) / sizeof(airlines[0]);

// Helper to get name from IATA/ICAO code
String getAirlineName(const char* code) {
  char buffer[40];  // buffer for PROGMEM string
  for (int i = 0; i < numAirlines; i++) {
    strcpy_P(buffer, (char*)pgm_read_word(&(airlines[i].icao)));
    if (strcmp(buffer, code) == 0) {
      strcpy_P(buffer, (char*)pgm_read_word(&(airlines[i].name)));
      return String(buffer);
    }
  }
  return String("Unknown Airline");
}

#endif
