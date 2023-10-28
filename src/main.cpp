#include <Arduino.h>
#include <dbg.h>
#include <Wire.h>
#include "apdsIf.h"
#include <Web.h>

// Global Variables
Web web=Web();

void setup()
{
    // Initialize Serial port
    Serial.begin(115200);
    ESP_LOGI(TAG,(" "));
    ESP_LOGI(TAG,("--------------------------------"));
    ESP_LOGI(TAG,("SparkFun APDS-9960 - GestureTest"));
    ESP_LOGI(TAG,("--------------------------------"));

    ESP_LOGI(TAG,("--Web setup---------------------"));
    web.setup();
    ESP_LOGI(TAG,("--------------------------------"));

    APDSIF_setup();

    // Assign callbacks of ADPS_IF to WEB-Server
    web.getApiSetupData = APDSIF_provideSensorSetupData;
    web.setApiSetupData = APDSIF_receiveSensorSetupData;
    web.setApiLLSetupData = APDSIF_receiveSensorLLSetupData;
    web.getApiOutputData = APDSIF_provideSensorOutputData;
}

void loop()
{
    APDSIF_loop();
    web.loop();
    delay(20);
    yield();
}
