# Hydroponic System Monitoring with ESP32

- **Google Sheets URL**: https://docs.google.com/spreadsheets/d/1dCgt7THF3cOpQ7jK7-KPpe2uO55g0JtL4vvuNqq9zh8/edit?gid=0#gid=0
- **Google Apps Script Manage URL**: https://script.google.com/u/0/home/projects/1poKho3ooc0cjWtJ61_2C0ZUHB_Q__r_TrzrOnwBDIZwgMc24Re1UqgjW/edit
- **Google Apps Script Webhook URL**: https://script.google.com/macros/s/AKfycbw-h2ujx4oPxS1luFL86c3IibBRv7MCIUbMsjPrtWk0OkXhXqD3NOIp8nsKQIlZHCM_1w/exec

## Project Overview

This project is designed to monitor environmental conditions in a hydroponic system using an ESP32 microcontroller. The system measures **temperature**, **humidity**, **light intensity**, and **water level** using a DHT11 sensor, LDR, and a water level sensor. The data is displayed on a TM1637 4-digit display, sent to a Google Sheet via Wi-Fi, and updated every 10 seconds.

## Components Used

- **ESP32 DEVKIT V1** - Microcontroller with Wi-Fi capability.
- **DHT11 Temperature and Humidity Sensor** - Measures ambient temperature and humidity.
- **TM1637 4-Digit 7-Segment Display** - Displays temperature and humidity.
- **LDR (Light Dependent Resistor)** - Measures light intensity.
- **Water Level Sensor** - Monitors the water level.
- **Google Sheets** - Stores and logs environmental data.
- **Google Apps Script** - Receives data from the ESP32 and writes it to Google Sheets.

## Features

- **Real-time Monitoring**: Temperature, humidity, light intensity, and water level are monitored every 10 seconds.
- **Data Logging to Google Sheets**: The ESP32 sends sensor data to Google Sheets via a webhook created in Google Apps Script.
- **Display Data Locally**: The TM1637 display shows the temperature (°C) and humidity (%) in integer format.

## Wiring and Setup

### ESP32 Pin Assignments

| Component                | ESP32 Pin |
| ------------------------ | --------- |
| **DHT11 (Data)**         | GPIO 5    |
| **TM1637 Display (CLK)** | GPIO 18   |
| **TM1637 Display (DIO)** | GPIO 19   |
| **LDR (Analog)**         | GPIO 34   |
| **Water Level Sensor**   | GPIO 36   |
| **VCC Connections**      | 3.3V      |
| **GND Connections**      | GND       |

### Wiring Instructions

1. **DHT11 Sensor**:

   - **Data Pin**: Connect to GPIO 5.
   - **VCC**: Connect to 3.3V.
   - **GND**: Connect to GND.

2. **TM1637 Display**:

   - **CLK**: Connect to GPIO 18.
   - **DIO**: Connect to GPIO 19.
   - **VCC**: Connect to 3.3V.
   - **GND**: Connect to GND.

3. **LDR Setup**:

   - Connect one side of the LDR to 3.3V.
   - Connect the other side of the LDR to GPIO 34.
   - Place a **10kΩ resistor** between GPIO 34 and GND to form a voltage divider.

4. **Water Level Sensor**:
   - **Analog Output**: Connect to GPIO 36.
   - **VCC**: Connect to 3.3V.
   - **GND**: Connect to GND.

## Software Setup

### Step 1: Prepare the Google Sheet and Apps Script

1. **Create a new Google Sheet**:

   - Name the sheet and create columns labeled: `Timestamp`, `Temperature`, `Humidity`, `Light Intensity`, `Water Level`.

2. **Create Google Apps Script**:

   - Go to **Extensions > Apps Script** in Google Sheets.
   - Paste the following script:

     ```javascript
     function doPost(e) {
       const sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
       const data = JSON.parse(e.postData.contents);
       sheet.appendRow([
         new Date(),
         data.temperature,
         data.humidity,
         data.light,
         data.water,
       ]);
       return ContentService.createTextOutput("Data added successfully");
     }
     ```

   - **Deploy as a Web App**:
     - Go to **Deploy > Manage deployments**, create a new deployment as a **Web App**.
     - Set access to **Anyone with the link** and copy the URL. This is your webhook URL.

3. **Add Webhook URL to ESP32 Code**:
   - Paste the URL into the ESP32 code as `webhookURL`.

### Step 2: Program the ESP32

1. **Install Libraries**:

   - In Arduino IDE, go to **Library Manager** and install the following:
     - `DHT sensor library` by Adafruit.
     - `TM1637Display` library for controlling the TM1637.

2. **Code for ESP32**

   Upload the following code to the ESP32:

   ```cpp
   #include <WiFi.h>
   #include <HTTPClient.h>
   #include <DHT.h>
   #include <TM1637Display.h>

   const char* ssid = "YOUR_SSID";
   const char* password = "YOUR_PASSWORD";
   const char* webhookURL = "YOUR_GOOGLE_SHEETS_WEBHOOK_URL";

   #define DHTPIN 5
   #define DHTTYPE DHT11
   DHT dht(DHTPIN, DHTTYPE);
   #define CLK 18
   #define DIO 19
   TM1637Display display(CLK, DIO);
   #define LDR_PIN 34
   #define R_FIXED 10000
   #define WATER_LEVEL_PIN 36

   void setup() {
     Serial.begin(115200);
     WiFi.begin(ssid, password);
     while (WiFi.status() != WL_CONNECTED) {
       delay(1000);
       Serial.println("Connecting to WiFi...");
     }
     dht.begin();
     display.setBrightness(0x0f);
     Serial.println("Starting sensor tests...");
   }

   void loop() {
     float h = dht.readHumidity();
     float t = dht.readTemperature();
     int lightIntensityRaw = analogRead(LDR_PIN);
     int waterLevelRaw = analogRead(WATER_LEVEL_PIN);

     float ldrResistance = (float)R_FIXED * (1023.0 / lightIntensityRaw - 1);
     float lux = 500 / (ldrResistance / 1000);

     if (isnan(h) || isnan(t)) {
       Serial.println("Failed to read from DHT sensor!");
       display.showNumberDecEx(0b1111, 0b01000000);
     } else {
       Serial.print("Temperature: ");
       Serial.print(t, 1);
       Serial.print(" °C, Humidity: ");
       Serial.print(h, 1);
       Serial.print(" %, Light Intensity: ");
       Serial.print(lux);
       Serial.print(" lux, Water Level: ");
       Serial.println(waterLevelRaw);

       displayTemperatureAndHumidity(round(t), round(h));

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
           Serial.println("Data sent successfully");
         } else {
           Serial.print("Error sending data: ");
           Serial.println(httpResponseCode);
         }
         http.end();
       }
     }
     delay(10000);
   }

   void displayTemperatureAndHumidity(int temp, int humidity) {
     int displayValue = (temp * 100) + humidity;
     display.showNumberDecEx(displayValue, 0b11100000, true);
   }
   ```
