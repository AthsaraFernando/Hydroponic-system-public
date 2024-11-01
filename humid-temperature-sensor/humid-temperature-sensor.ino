/*
   This code reads temperature and humidity data from a DHT11 sensor 
   and displays the rounded integer values on a TM1637 4-digit 7-segment display.

   - DHT11 sensor setup:
     - Data pin connected to Arduino Pin 7.
     - Provides temperature in °C and humidity as a percentage (%).

   - TM1637 4-digit display setup (model 3462BS):
     - CLK (clock) pin connected to Arduino Pin 10.
     - DIO (data) pin connected to Arduino Pin 11.

   Functionality:
   - The first two digits of the display show the temperature in °C as an integer.
   - The last two digits show the humidity in % as an integer.
   - The middle colon (two dots) on the display is always on.
   
   Additional:
   - Precise temperature and humidity (with one decimal point) are printed to the Serial Monitor.
   - Readings update every 10 seconds.
*/

#include <DHT.h>
#include <TM1637Display.h>

// DHT11 Configuration
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// TM1637 Display Configuration
#define CLK 10
#define DIO 11
TM1637Display display(CLK, DIO);

void setup() {
  Serial.begin(9600);
  dht.begin();
  display.setBrightness(0x0f); // Max brightness for TM1637
  Serial.println("Starting DHT11 sensor test...");
  delay(2000);  // Add delay for initialization
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    display.showNumberDecEx(0b1111, 0b01000000); // Display ---- on error
  } else {
    // Print original values with decimals to Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(t, 1);  // Show temperature with one decimal place
    Serial.print(" °C, Humidity: ");
    Serial.print(h, 1);  // Show humidity with one decimal place
    Serial.println(" %");

    // Display rounded integer values for temperature and humidity on TM1637
    displayTemperatureAndHumidity(round(t), round(h));
  }

  delay(10000); // 10-minute delay between readings
}

// Function to display temperature and humidity as integers with middle dots on TM1637
void displayTemperatureAndHumidity(int temp, int humidity) {
  // Combine temperature and humidity as a 4-digit number (e.g., 2516 for 25°C, 16%)
  int displayValue = (temp * 100) + humidity;
  display.showNumberDecEx(displayValue, 0b11100000, true); // Display in format TT:HH with middle colon on
}
