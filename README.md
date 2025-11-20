# ESP32 Flight Tracker

An ESP32-based flight tracker that retrieves live flight data using the AeroAPI (FlightAware) and displays relevant information.

## Features

* Tracks flights in real-time using your latitude and longitude.
* Filters flights to only show commercial aircraft.
* Displays flight information including origin, destination, airline, flight number, and aircraft type.
* Configurable via a `config.h` file for WiFi credentials and API key.

## Technologies Used

* ESP32
* HUB75 RGB LED Matrix Panel
* OpenSky API
* AeroAPI (FlightAware)

## Setup

1. **Clone the repository**

```bash
git clone https://github.com/yourusername/esp32-flight-tracker.git
cd esp32-flight-tracker
```

2. **Install Arduino libraries**

* WiFi.h
 – for WiFi connectivity
* WiFiClientSecure.h
 – for HTTPS requests
* HTTPClient.h
 – for HTTP requests
* ArduinoJson.h
 – for JSON parsing
* ESP32-HUB75-MatrixPanel-I2S-DMA
 – for driving the LED matrix pane

3. **Create `config.h`**
   Create a file called `config.h` in the project folder with the following content:

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials
const char* SSID = "YourWiFiSSID";
const char* PASSWORD = "YourWiFiPassword";

// AeroAPI (FlightAware) key
const char* AEROAPIKEY = "YourAPIKey";

float LAT = your_latitude;
float LON = your_longitude;

#endif
```

> ⚠️ Do **not** upload your `config.h` to public repositories.

4. **Upload the code to ESP32**

* Open the project in Arduino IDE.
* Select your ESP32 board and COM port.
* Compile and upload.

## Usage

1. Power the ESP32 and connect it to WiFi.
2. The ESP32 will automatically fetch flight data near your configured location.
3. Only commercial flights will be displayed.
