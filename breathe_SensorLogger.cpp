/***********************************************************************
 * FILE NAME: SensorLogger.cpp
 *
 * PURPOSE: Contains the implementation for logging sensor data to an SD 
 * card in CSV format. It retrieves data from the sensors and logs it.
 *
 * DEVELOPMENT HISTORY:
 *
 * Date              | Name                | Change ID | Release Description
 * ------------------|---------------------|-----------|----------------------
 * 20-03-2025        | diego.martinez      | change_id | Initial commit
 ***********************************************************************/

/* ---------------------------- Includes ---------------------------- */
#include "breathe_SensorLogger.h"
#include <Wire.h>
#include <SD.h>
#include <time.h>

/* ---------------------------- Externs ---------------------------- */

/* ---------------------------- Static Variables ---------------------------- */

/* ---------------------------- Global Functions ---------------------------- */

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
 * FUNCTION NAME: logSensorData
 *
 * @brief Logs sensor data to the SD card in CSV format.
 *
 ***********************************************************************/
void logSensorData() {
  // Get the current timestamp from RTC time
  time_t currentTime = getCurrentTime();
  struct tm* timeinfo = localtime(&currentTime);

  char timestamp[30];  // Buffer for formatted time
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

  // Open the CSV file in append mode
  File dataFile = SD.open(DATA_FILE_PATH, FILE_APPEND);
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
