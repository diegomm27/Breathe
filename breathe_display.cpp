/***********************************************************************
 *
 * FILE NAME: breathe_display.c
 *
 * PURPOSE: Provides functions to display menu and sensor data on M5Stack screen.
 *
 * DEVELOPMENT HISTORY:
 *
 * Date              | Name                | Change ID | Release Description
 * ------------------|---------------------|-----------|----------------------
 * date              | diego.martinez      | change_id | initial release
 ***********************************************************************/

#include "breathe_display.h"

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
  M5.Lcd.setCursor(229, 225);  // Adjust the cursor position for 'freq'
  M5.Lcd.printf(".Freq.");
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

  // Update this section to use timeIntervals[]
  int currentInterval = timeInterval;  // Set the interval to the first value, or select another index

  M5.Lcd.setTextColor(TFT_YELLOW);
  M5.Lcd.setCursor(10, 80);
  M5.Lcd.printf("%d ms", currentInterval);  // Display the current selected interval

  // Draw a simple bar representation (update only the bar)
  int barWidth = map(timeInterval, 100, 5000, 10, 220);  // Scale between min/max interval
  M5.Lcd.fillRect(10, 120, barWidth, 20, TFT_GREEN);

  createmenu();
}


