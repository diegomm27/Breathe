/***********************************************************************
 * FILE NAME: SensorLogger.h
 *
 * PURPOSE: Contains functions to log sensor data to an SD card in CSV 
 * format, along with functions to manage sensor data.
 *
 * DEVELOPMENT HISTORY:
 *
 * Date              | Name                | Change ID | Release Description
 * ------------------|---------------------|-----------|----------------------
 * 20-03-2025        | diego.martinez      | change_id | Initial commit
 ***********************************************************************/

#ifndef SENSORLOGGER_H
#define SENSORLOGGER_H

/* ---------------------------- Includes ---------------------------- */
#include <SD.h>
#include <Wire.h>
#include "SHT3X.h"
#include "SPS30.h"
#include "QMP6988.h"

/* ---------------------------- Macros and Defines ---------------------------- */
#define DATA_FILE_PATH "/data.csv"

/* ---------------------------- External Variables ---------------------------- */
extern float pm1;
extern float pm25;
extern float pm4;
extern float pm10;
extern float avp;
extern float envTemp;
extern float envHum;
extern float envPressure;
extern float envAltitude;

extern time_t epochTime;  // This will link to the epochTime variable in WiFiUtils.cpp
extern uint32_t startTime;
/* ---------------------------- Global Functions Definitions ---------------------------- */
void logSensorData();
time_t getCurrentTime();

#endif /* SENSORLOGGER_H */
