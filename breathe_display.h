#ifndef BREATHE_DISPLAY_H
#define BREATHE_DISPLAY_H

/* --- Includes --- */
#include <M5Stack.h>

extern float temp;
extern float hum;
extern float pm1;
extern float pm25;
extern float pm4;
extern float pm10;
extern float avp;
extern float envTemp;
extern float envHum;
extern float envPressure;
extern float envAltitude;

extern const int timeIntervals[];
extern int timeInterval;

/* --- Function Prototypes --- */
void createmenu(void);
void displayInfo(void);
void displayCfg(void);

#endif /* BREATHE_DISPLAY_H */
