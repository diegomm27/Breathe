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
#include "sensirion_uart.h"

#include "breathe_WiFiUtils.h"
#include "breathe_SensorLogger.h"
#include "breathe_display.h"

/* ---------------------------- Macros and Defines ---------------------------- */
#define PASCAL_TO_ATM 0.00000986923  // Pascal to Atmospheres conversion factor

/* ---------------------------- Static Variables ---------------------------- */
DHT12 dht12;
SHT3X sht3x;
QMP6988 qmp;
File dataFile;

byte dmode = 0;  // Default display mode: 0 = info, 1 = temp & hum, 2 = PM

// Sensor readings (no longer needed in this scope)
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

uint32_t startTime = 0;  // Stores millis() at sync

const int timeIntervals[] = { 100, 500, 1000, 2000, 3000, 4000, 5000 };
const int numIntervals = sizeof(timeIntervals) / sizeof(timeIntervals[0]);
int currentIntervalIndex = 2;                            // Default index for 1000ms
int timeInterval = timeIntervals[currentIntervalIndex];  // Initialize timeInterval

String wifiSsid = "";
String wifiPassword = "";

/* ---------------------------- Function Prototypes ---------------------------- */

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

  // Initialize SD card
  if (!SD.begin()) {
    Serial.println("⚠️ SD Card initialization failed!");
    while (1);  // Infinite loop if SD card initialization fails
  }

  // Load WiFi credentials from config file
  if (loadConfig(wifiSsid, wifiPassword)) {
    Serial.println("✅ Wi-Fi credentials loaded successfully!");
    Serial.print("SSID: ");
    Serial.println(wifiSsid);
  } else {
    Serial.println("⚠️ Unable to load Wi-Fi credentials.");
    wifiSsid = "defaultSSID";
    wifiPassword = "defaultPASSWORD";
  }

  // Connect to WiFi and get time
  WiFi.begin(wifiSsid, wifiPassword);
  connectWiFiAndGetTime();

  // Initialize SPS30 sensor
  while (sps30_probe() != 0) {
    Serial.println("Probe failed");
    delay(500);  // Retry if the probe fails
  }
  sps30_set_fan_auto_cleaning_interval(15 * 60);  // Set auto cleaning interval for SPS30
  sps30_start_measurement();  // Start the SPS30 measurement

  // Initialize I2C sensors
  qmp.begin(&Wire, QMP6988_SLAVE_ADDRESS_L, 21, 22, 400000U);  // QMP6988 initialization
  sht3x.begin(&Wire, SHT3X_I2C_ADDR, 21, 22, 400000U);  // SHT3X initialization

  // Create data.csv if it doesn't exist
  if (!SD.exists("/data.csv")) {
    dataFile = SD.open("/data.csv", FILE_WRITE);
    if (dataFile) {
      // Write the header for the CSV file
      dataFile.println("Timestamp,SPS30_PM1_0,SPS30_PM2_5,SPS30_PM4_0,SPS30_PM10_0,"
                       "SPS30_Avg_Particle_Size,SHT3X_Env_Temperature_C,"
                       "SHT3X_Env_Humidity_Percent,QMP6988_Env_Pressure_atm,"
                       "QMP6988_Env_Altitude_m");
      dataFile.close();
    } else {
      Serial.println("⚠️ Error opening data.csv for writing.");
    }
  }

  // Optional: Print initialization success message
  Serial.println("✅ Setup complete, sensors initialized, and data logging ready.");
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

    logSensorData();  // This will read data from sensors and log it

    // Update the last time the data was acquired
    lastMillis = currentMillis;
  }
}
