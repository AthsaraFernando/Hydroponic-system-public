/*
   This code monitors temperature, humidity, light intensity, and water level
   in a hydroponic system using various sensors connected to an ESP32.

   - DHT11 sensor setup:
     - Data pin connected to GPIO 5.
     - Measures temperature in °C and humidity as a percentage (%).

   - TM1637 4-digit display setup (model 3462BS):
     - CLK (clock) pin connected to GPIO 18.
     - DIO (data) pin connected to GPIO 19.
     - First two digits display temperature (°C) as an integer.
     - Last two digits display humidity (%) as an integer.
     - Middle colon (two dots) on the display is always on.

   - LDR (Light Dependent Resistor) setup for light intensity:
     - LDR connected between 3.3V and GPIO 34 with a 10kΩ resistor to GND.
     - The analog reading is converted to an estimated lux value.

   - Water Level Sensor setup:
     - Analog out pin connected to GPIO 36.
     - Reads water level as an analog value (0-1023), with 0 meaning low level 
       and 1023 meaning high level.

   - Internet Connection and Google Sheets Logging:
     - The ESP32 connects to Wi-Fi and sends data to Google Sheets using an HTTP POST request.
     - Data includes temperature, humidity, light intensity, and water level.

   Functionality:
   - The TM1637 display shows integer values of temperature and humidity.
   - The Serial Monitor outputs precise temperature and humidity, light intensity in lux, 
     and water level as a raw analog value.
   - Readings update every 10 seconds and are sent to Google Sheets for tracking.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <TM1637Display.h>

// WiFi credentials
const char* ssid = "XXXreplaceXXX";
const char* password = "XXXreplaceXXX";

// Google Sheets webhook URL
const char* webhookURL = "XXXreplaceXXX";

// DHT11 Configuration
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// TM1637 Display Configuration
#define CLK 18
#define DIO 19
TM1637Display display(CLK, DIO);

// LDR Configuration
#define LDR_PIN 34
#define R_FIXED 10000 // Fixed resistor value in ohms (10kΩ)

// Water Level Sensor Configuration
#define WATER_LEVEL_PIN 36

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  dht.begin();
  display.setBrightness(0x0f); // Max brightness for TM1637
  Serial.println("Starting sensor tests...");
  delay(2000);  // Add delay for initialization
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int lightIntensityRaw = analogRead(LDR_PIN); // Read light intensity from LDR
  int waterLevelRaw = analogRead(WATER_LEVEL_PIN); // Read water level from the sensor

  // Calculate LDR resistance
  float ldrResistance = (float)R_FIXED * (1023.0 / lightIntensityRaw - 1);

  // Approximate Lux calculation
  float lux = 500 / (ldrResistance / 1000); // Convert ohms to kΩ for lux approximation

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    display.showNumberDecEx(0b1111, 0b01000000); // Display ---- on error
  } else {
    // Print original values with decimals to Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(t, 1);  // Show temperature with one decimal place
    Serial.print(" °C, Humidity: ");
    Serial.print(h, 1);  // Show humidity with one decimal place
    Serial.print(" %, Light Intensity: ");
    Serial.print(lux);
    Serial.print(" lux, Water Level: ");
    Serial.println(waterLevelRaw); // Print raw water level reading

    // Display rounded integer values for temperature and humidity on TM1637
    displayTemperatureAndHumidity(round(t), round(h));

    // Send data to Google Sheets via HTTP POST
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(webhookURL);
      http.addHeader("Content-Type", "application/json");

      String jsonData = "{\"temperature\":" + String(t, 1) + 
                        ", \"humidity\":" + String(h, 1) + 
                        ", \"light\":" + String(lux) + 
                        ", \"water\":" + String(waterLevelRaw) + "}";

      int httpResponseCode = http.POST(jsonData);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Data sent successfully");
        Serial.println(response);
      } else {
        Serial.print("Error sending data: ");
        Serial.println(httpResponseCode);
      }

      http.end();
    } else {
      Serial.println("WiFi not connected");
    }
  }

  delay(10000); // 10-second delay between readings
}

// Function to display temperature and humidity as integers with middle dots on TM1637
void displayTemperatureAndHumidity(int temp, int humidity) {
  // Combine temperature and humidity as a 4-digit number (e.g., 2516 for 25°C, 16%)
  int displayValue = (temp * 100) + humidity;
  display.showNumberDecEx(displayValue, 0b11100000, true); // Display in format TT:HH with middle colon on
}
