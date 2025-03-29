/***********************************************************************
 * FILE NAME: WiFiUtils.cpp
 *
 * PURPOSE: Contains the implementation of WiFiUtils functions to load 
 * WiFi credentials, connect to Wi-Fi, and retrieve the current time 
 * from an NTP server.
 *
 * DEVELOPMENT HISTORY:
 *
 * Date              | Name                | Change ID | Release Description
 * ------------------|---------------------|-----------|----------------------
 * 20-03-2025        | diego.martinez      | change_id | Initial commit
 ***********************************************************************/

/* ---------------------------- Includes ---------------------------- */
#include "breathe_WiFiUtils.h"
#include <Wire.h>
#include <WiFi.h>
#include <SD.h>

/* ---------------------------- Static Variables ---------------------------- */

time_t epochTime = 0;    // Stores the NTP time

/* ---------------------------- Static Function Prototypes ---------------------------- */

/* ---------------------------- Global Functions ---------------------------- */

/***********************************************************************
 *
 * FUNCTION NAME: loadConfig
 *
 * @brief Reads WiFi credentials from the config file on the SD card.
 *
 * ARGUMENT LIST:
 * - String &ssid: Reference to store the WiFi SSID.
 * - String &password: Reference to store the WiFi password.
 *
 * RETURN VALUE: bool - true if successfully loaded, false otherwise.
 *
 ***********************************************************************/
bool loadConfig(String &ssid, String &password) {
  File configFile = SD.open(CONFIG_FILE_PATH, FILE_READ);
  if (!configFile) {
    Serial.println("⚠️ Failed to open config file!");
    return false;
  }

  bool credentialsSection = false;  // Track if we're in the correct section
  
  while (configFile.available()) {
    String line = configFile.readStringUntil('\n');
    line.trim();  // Remove spaces and newlines

    if (line == "[WIFI_CREDENTIALS]") {
      credentialsSection = true;  // We are now in the Wi-Fi credentials section
      continue;
    }

    if (credentialsSection) {
      if (line.startsWith("SSID=")) {
        ssid = line.substring(5);  // Extract SSID after "SSID="
        Serial.print("SSID Found: ");
        Serial.println(ssid);  // Print the extracted SSID
      } else if (line.startsWith("PASSWORD=")) {
        password = line.substring(9);  // Extract Password after "PASSWORD="
        Serial.print("Password Found: ");
        Serial.println(password);  // Print the extracted Password
      }
    }
  }

  configFile.close();

  // Check if ssid and password were filled
  if (ssid.length() > 0 && password.length() > 0) {
    Serial.println("Successfully loaded Wi-Fi credentials.");
    return true;  // Successfully loaded Wi-Fi credentials
  } else {
    Serial.println("⚠️ WiFi credentials not found in config file!");
    return false;  // Credentials not found
  }
}


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
