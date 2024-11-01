#include <DHT.h>
#include <TM1637Display.h>

// DHT11 Configuration
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// TM1637 Configuration for Humidity Display //
#define CLK 10
#define DIO 11
TM1637Display display(CLK, DIO);

// 7-Segment Display Pins (Temperature)
int tempSegmentPins[] = {2, 3, 4, 5, 6, 7, 8, 9};

// Define segments for each digit (0-9) and the letter 'C'
const byte digits[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};
const byte segmentC[7] = {1, 0, 0, 1, 1, 1, 0}; // 'C' character

void setup() {
  Serial.begin(9600);
  dht.begin();
  display.setBrightness(0x0f); // Max brightness for TM1637

  // Set all pins for the 8-pin 7-segment as outputs
  for (int i = 0; i < 8; i++) {
    pinMode(tempSegmentPins[i], OUTPUT);
  }

  Serial.println("Starting DHT11 sensor test...");
  delay(2000);
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    // Display ---- on both displays
    display.showNumberDecEx(0b1111, 0b01000000); // TM1637: ----
    displayTemperatureError(); // 8-pin 7-segment: ----
  } else {
    // Display Temperature on 8-pin 7-segment display
    displayTemperature(t);

    // Display Humidity on TM1637 display
    displayHumidity(h);
  }

  delay(2000);
}

// Function to display temperature on the 8-pin 7-segment display
void displayTemperature(float temp) {
  int wholePart = (int)temp;
  int decimalPart = (int)((temp - wholePart) * 10);

  displayDigit(wholePart / 10, 0); // Tens
  displayDigit(wholePart % 10, 1); // Units
  displayDigit(decimalPart, 2);    // Decimal
  displaySegment(segmentC, 3);     // 'C'
}

// Function to display a single digit on the 8-pin 7-segment display
void displayDigit(int num, int digitPos) {
  if (num < 0 || num > 9) return;
  displaySegment(digits[num], digitPos);
}

// Function to turn on the specific segments for a digit
void displaySegment(const byte seg[7], int digitPos) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(tempSegmentPins[digitPos * 7 + i], seg[i]);
  }
}

// Function to display humidity on the TM1637 display
void displayHumidity(float humidity) {
  int humInt = (int)humidity;
  display.showNumberDec(humInt, false, 2, 0); // Show only first 2 digits
}

// Function to display ---- on the 8-pin 7-segment display
void displayTemperatureError() {
  byte errorSegments[7] = {1, 0, 0, 1, 1, 0, 1}; // For '----'
  for (int i = 0; i < 4; i++) {
    displaySegment(errorSegments, i);
  }
}
