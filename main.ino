/***********************************************************************
 *
 * FILE NAME: m5stack_sensor_display.ino
 *
 * PURPOSE: This program reads data from various sensors (SPS30, SHT3X, QMP6988) 
 * and displays the information on an M5Stack screen. It also logs the 
 * sensor data to an SD card.
 *
 * DEVELOPMENT HISTORY:
 *
 * Date                 Name                 Change ID           Release Description
 * --------       ------------------          -------        ---------------------------
 * 20-03-2025       Diego Martínez                                 Initial commit
 *
 ***********************************************************************/

/* ---------------------------- Includes ---------------------------- */
#include <M5Stack.h>
#include "DHT12.h"
#include <Wire.h>
#include "sensirion_uart.h"
#include "sps30.h"
#include <SD.h>
#include "QMP6988.h"
#include "SHT3X.h"
#include <WiFi.h>


/* ---------------------------- Macros and Defines ---------------------------- */
#define PASCAL_TO_ATM 0.00000986923  // Pascal to Atmospheres conversion factor

/* ---------------------------- Static Variables ---------------------------- */
DHT12 dht12;
SHT3X sht3x;
QMP6988 qmp;
File dataFile;

byte dmode = 0;  // Default display mode: 0 = info, 1 = temp & hum, 2 = PM

// Sensor readings
float temp = 0;
float hum = 0;
float pm1 = 0;
float pm25 = 0;
float pm4 = 0;
float pm10 = 0;
float avp = 0;
float envTemp = 0;
float envHum = 0;
float envPressure = 0;
float envAltitude = 0;

unsigned long lastMillis = 0;             // Store last update time
unsigned long lastButtonPress = 0;        // For debouncing
const unsigned long debounceDelay = 200;  // Debounce delay in ms

time_t epochTime = 0;    // Stores the NTP time
uint32_t startTime = 0;  // Stores millis() at sync

const char* wifiSsid = "DIGIFIBRA-9UkK";
const char* wifiPassword = "UPE7D7ZCK7K2";

const int timeIntervals[] = { 100, 500, 1000, 2000, 3000, 4000, 5000 };
const int numIntervals = sizeof(timeIntervals) / sizeof(timeIntervals[0]);
int currentIntervalIndex = 2;                            // Default index for 1000ms
int timeInterval = timeIntervals[currentIntervalIndex];  // Initialize timeInterval

/* ---------------------------- Function Prototypes ---------------------------- */
void connectWiFiAndGetTime(void);
time_t getCurrentTime(void);
void createmenu(void);
void displayInfo(void);
void displayCfg(void);
void logSensorData(void);


/***********************************************************************
 *
 * FUNCTION NAME: connectWiFiAndGetTime
 *
 * @brief Connects to Wi-Fi, retrieves the current time via NTP, and disconnects.
 *
 ***********************************************************************/

void connectWiFiAndGetTime() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(wifiSsid, wifiPassword);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected!");

    configTime(0, 0, "pool.ntp.org");  // Get time from NTP server
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      epochTime = mktime(&timeinfo);  // Convert to Unix timestamp
      startTime = millis();           // Store millis() at sync time
      Serial.println("Time obtained successfully.");
      Serial.print("Epoch Time: ");
      Serial.println(epochTime);
    } else {
      Serial.println("Failed to obtain time.");
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("Wi-Fi Disconnected.");
  } else {
    Serial.println("Failed to connect to Wi-Fi.");
  }
}


/***********************************************************************
 *
 * FUNCTION NAME: getCurrentTime
 *
 * @brief Returns the current Unix timestamp, adjusted for elapsed time.
 *
 * RETURN VALUE: time_t - Current Unix timestamp.
 *
 ***********************************************************************/
time_t getCurrentTime() {
  return epochTime + ((millis() - startTime) / 1000);
}


/***********************************************************************
 *
 * FUNCTION NAME: createmenu
 *
 * @brief Displays the menu options on the M5Stack screen.
 *
 ***********************************************************************/
void createmenu() {
  M5.Lcd.setCursor(35, 225);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.printf(".Data.");
  M5.Lcd.setCursor(132, 225);
  M5.Lcd.printf(".Cfg.");
}


/***********************************************************************
 *
 * FUNCTION NAME: displayInfo
 *
 * @brief Updates the M5Stack display with the latest sensor data.
 *
 ***********************************************************************/
void displayInfo() {
  M5.Lcd.fillScreen(TFT_BLACK);  // Clear the screen first
  M5.Lcd.setTextSize(1.8);
  int y = 10;

  M5.Lcd.setTextColor(TFT_CYAN);
  M5.Lcd.setCursor(10, y);
  M5.Lcd.print("SPS30 Sensor:");
  y += 30;

  M5.Lcd.setTextColor(TFT_YELLOW);
  M5.Lcd.setCursor(10, y);
  M5.Lcd.printf("PM1.0: %.1f  PM2.5: %.1f  PM4.0: %.1f  PM10: %.1f", pm1, pm25, pm4, pm10);
  y += 30;
  M5.Lcd.setCursor(10, y);
  M5.Lcd.printf("Avg Particle Size: %.1f", avp);
  y += 30;

  M5.Lcd.setTextColor(TFT_CYAN);
  M5.Lcd.setCursor(10, y);
  M5.Lcd.print("Environmental Data:");
  y += 30;

  M5.Lcd.setTextColor(TFT_YELLOW);
  M5.Lcd.setCursor(10, y);
  M5.Lcd.printf("Temp: %.1f C  Hum: %.1f%%", envTemp, envHum);
  y += 30;
  M5.Lcd.setCursor(10, y);
  M5.Lcd.printf("Pressure: %.6f atm  Altitude: %.2f m", envPressure, envAltitude);

  // ---------- SD Storage Info ----------
  uint64_t totalBytes = SD.totalBytes();
  uint64_t usedBytes = SD.usedBytes();
  uint64_t freeBytes = totalBytes - usedBytes;
  float freePercent = (totalBytes > 0) ? (((float)freeBytes / (float)totalBytes) * 100.0f) : 0.0f;

  y += 30;

  M5.Lcd.setTextColor(TFT_ORANGE);
  M5.Lcd.setCursor(10, y);
  M5.Lcd.print("SD Storage:");
  M5.Lcd.setTextColor(TFT_GREEN);
  M5.Lcd.printf("%.1f%% free", freePercent);

  createmenu();
}

/***********************************************************************
 *
 * FUNCTION NAME: displayCfg
 *
 * @brief Displays the configuration tab with the current time interval.
 *
 ***********************************************************************/
void displayCfg() {
  M5.Lcd.fillScreen(TFT_BLACK);  // Clear the screen first
  M5.Lcd.setTextSize(1.8);

  M5.Lcd.setTextColor(TFT_CYAN);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print("Configuration Tab");

  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setCursor(10, 50);
  M5.Lcd.print("Sampling Interval (ms):");

  // Only update this section to avoid flickering
  M5.Lcd.setTextColor(TFT_YELLOW);
  M5.Lcd.setCursor(10, 80);
  M5.Lcd.printf("%d ms", timeInterval);

  // Draw a simple bar representation (update only the bar)
  int barWidth = map(timeInterval, 100, 5000, 10, 220);  // Scale between min/max interval
  M5.Lcd.fillRect(10, 120, barWidth, 20, TFT_GREEN);

  createmenu();
}


/***********************************************************************
 *
 * FUNCTION NAME: logSensorData
 *
 * @brief Logs sensor data to the SD card in CSV format
 *
 ***********************************************************************/
void logSensorData() {
  // Get the current timestamp from RTC time
  time_t currentTime = getCurrentTime();
  struct tm* timeinfo = localtime(&currentTime);

  char timestamp[30];  // Buffer for formatted time
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

  // Open the CSV file in append mode
  File dataFile = SD.open("/data.csv", FILE_APPEND);
  if (dataFile) {
    // Write timestamp and sensor data in CSV format
    dataFile.printf("%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                    timestamp, pm1, pm25, pm4, pm10, avp, envTemp, envHum, envPressure, envAltitude);
    dataFile.close();
    Serial.println("Data logged successfully.");
  } else {
    Serial.println("Error opening data.csv for appending.");
  }

  // Optional: Print SD card usage
  uint64_t totalBytes = SD.totalBytes();
  uint64_t usedBytes = SD.usedBytes();
  Serial.printf("SD Usage: %llu/%llu bytes\n", usedBytes, totalBytes);
}


/***********************************************************************
 *
 * FUNCTION NAME: setup
 *
 * @brief Initializes sensors, SD card, and starts measurement.
 *
 ***********************************************************************/
void setup() {
  M5.begin();
  Wire.begin();
  Serial.begin(230400);
  sensirion_uart_open();

  connectWiFiAndGetTime();

  while (sps30_probe() != 0) {
    Serial.println("Probe failed");
    delay(500);
  }
  sps30_set_fan_auto_cleaning_interval(15 * 60);
  sps30_start_measurement();

  if (!SD.begin()) {
    Serial.println("SD Card initialization failed!");
    while (1)
      ;
  }

  qmp.begin(&Wire, QMP6988_SLAVE_ADDRESS_L, 21, 22, 400000U);
  sht3x.begin(&Wire, SHT3X_I2C_ADDR, 21, 22, 400000U);

  if (!SD.exists("/data.csv")) {
    // Open the CSV file for writing
    dataFile = SD.open("/data.csv", FILE_WRITE);
    if (dataFile) {
      // Write the header with timestamp as the first column
      dataFile.println("Timestamp,SPS30_PM1_0,SPS30_PM2_5,SPS30_PM4_0,SPS30_PM10_0,SPS30_Avg_Particle_Size,SHT3X_Env_Temperature_C,SHT3X_Env_Humidity_Percent,QMP6988_Env_Pressure_atm,QMP6988_Env_Altitude_m");
      dataFile.close();
    } else {
      Serial.println("Error opening data.csv for writing.");
    }
  }
}

/***********************************************************************
 *
 * FUNCTION NAME: loop
 *
 * @brief Reads sensor data and updates the display periodically.
 *
 ***********************************************************************/

void loop() {
  M5.update();
  unsigned long currentMillis = millis();

  // Button event handling (with debouncing)
  if (M5.BtnA.wasReleased()) {
    Serial.println("button A");
    dmode = 0;  // Switch to Info mode
  } else if (M5.BtnB.wasReleased()) {
    Serial.println("button B");
    dmode = 1;  // Switch to Config mode
  } else if (M5.BtnC.wasReleased()) {
    // Check if enough time has passed for debouncing
    if (currentMillis - lastButtonPress >= debounceDelay) {
      // Cycle through timeInterval values
      currentIntervalIndex = (currentIntervalIndex + 1) % numIntervals;
      timeInterval = timeIntervals[currentIntervalIndex];  // Update timeInterval

      // Refresh the screen if in the Cfg tab
      if (dmode == 1) {
        displayCfg();
      }

      // Update lastButtonPress time
      lastButtonPress = currentMillis;
    }
  }

  // Mode select and display based on dmode value
  if (dmode == 0) {
    displayInfo();
  } else if (dmode == 1) {
    displayCfg();
  }

  // Check if enough time has passed based on the timeInterval for sensor data update
  if (currentMillis - lastMillis >= timeInterval) {
    // Update sensor data only if the time interval has passed
    struct sps30_measurement measurement;

    if (sps30_read_measurement(&measurement) >= 0) {
      temp = dht12.readTemperature();
      hum = dht12.readHumidity();
      pm1 = measurement.mc_1p0;
      pm25 = measurement.mc_2p5;
      pm4 = measurement.mc_4p0;
      pm10 = measurement.mc_10p0;
      avp = measurement.typical_particle_size;
    } else {
      Serial.println("Read measurement failed");
    }

    if (sht3x.update() && qmp.update()) {
      envTemp = sht3x.cTemp;
      envHum = sht3x.humidity;
      envPressure = qmp.pressure * PASCAL_TO_ATM;
      envAltitude = qmp.altitude;
      logSensorData();  // Log data with timestamp
    } else {
      Serial.println("Sensor data update failed.");
    }

    // Update the last time the data was acquired
    lastMillis = currentMillis;
  }
}
