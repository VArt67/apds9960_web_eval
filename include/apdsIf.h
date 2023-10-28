#ifndef __APDSIF_H__
#define __APDSIF_H__

#include <ArduinoJson.h>

extern void APDSIF_setup();
extern void APDSIF_loop();

extern void APDSIF_provideSensorSetupData(DynamicJsonDocument &doc);
extern void APDSIF_receiveSensorSetupData(DynamicJsonDocument &doc);
extern void APDSIF_receiveSensorLLSetupData(DynamicJsonDocument &doc);
extern void APDSIF_provideSensorOutputData(DynamicJsonDocument &doc);

#endif // __APDSIF_H__
