/***********************************************************************
 * FILE NAME: WiFiUtils.h
 *
 * PURPOSE: Contains functions for loading WiFi credentials from the 
 * config file and connecting to Wi-Fi. It also retrieves time from 
 * an NTP server.
 *
 * DEVELOPMENT HISTORY:
 *
 * Date              | Name                | Change ID | Release Description
 * ------------------|---------------------|-----------|----------------------
 * 20-03-2025        | diego.martinez      | change_id | Initial commit
 ***********************************************************************/

#ifndef WIFIUTILS_H
#define WIFIUTILS_H

/* ---------------------------- Includes ---------------------------- */
#include <WiFi.h>
#include <SD.h>

/* ---------------------------- Macros and Defines ---------------------------- */
#define CONFIG_FILE_PATH "/config.cfg"  // Leading slash if the file is in the root directory of the SD card

/* ---------------------------- External Variables ---------------------------- */
extern String wifiSsid;
extern String wifiPassword;

extern uint32_t startTime;

/* ---------------------------- Global Functions Definitions ---------------------------- */
bool loadConfig(String &ssid, String &password);
void connectWiFiAndGetTime();

#endif /* WIFIUTILS_H */
