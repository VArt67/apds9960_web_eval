#include <Arduino.h>
#include <dbg.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <Web.h>
#include <apdsIf.h>

// Constants
// Pins on wemos D1 mini
#define APDS9960_SDA 4 // GPIO4 (D2)
#define APDS9960_SCL 5 // GPIO5 (D1)
const byte APDS9960_INT = 12; // GPIO12 (D6)

// Protos
static void interruptRoutine();
static void handleGesture();
static void handleALS();
static void handleProximity();


// Global Variables
static SparkFun_APDS9960 apds = SparkFun_APDS9960();

static uint16_t numInterrupts = 0;
static uint16_t numPolls = 0;
static bool pollEnabled = false;
static String ALS_output, Proxi_output, Gesture_output;

//----------------------------------------------------------------

static volatile uint8_t isr_flag = 0;

static void interruptRoutine()
{
    isr_flag = 1;
    ++numInterrupts;
}

// Interrupt callback function in IRAM
static void ICACHE_RAM_ATTR interruptRoutine();

//----------------------------------------------------------------

void APDSIF_setup()
{
    // Start I2C with pins defined above
    Wire.begin(APDS9960_SDA, APDS9960_SCL);

    // Set interrupt pin as input
    pinMode(digitalPinToInterrupt(APDS9960_INT), INPUT);

    // Initialize interrupt service routine
    attachInterrupt(digitalPinToInterrupt(APDS9960_INT), interruptRoutine, FALLING);

    // Initialize APDS-9960 (configure I2C and initial values)
    if (apds.init())
    {
        ESP_LOGI(TAG, ("APDS-9960 initialization complete"));
    }
    else
    {
        ESP_LOGE(TAG, ("Something went wrong during APDS-9960 init!"));
    }

    // Start running the APDS-9960 gesture sensor engine
    if (apds.enableGestureSensor(true))
    {
        ESP_LOGI(TAG, ("Gesture sensor is now running"));
    }
    else
    {
        ESP_LOGE(TAG, ("Something went wrong during gesture sensor init!"));
    }
}

void APDSIF_loop()
{
    if (isr_flag != 0)
    {
        apds.disableInterrupts();
        detachInterrupt(digitalPinToInterrupt(APDS9960_INT));
        isr_flag = 0;
        uint8_t status = apds.getStatusRegister();
        if (status != 0xFF) // not error
        {
            ESP_LOGD(TAG, "Interrupt 0x%x", status);
            if (status & 0x04) // GINT from statusReg
            {
                ESP_LOGD(TAG, "Gesture interrupt");
                handleGesture();
            }
            /*
            else
            {
                // We can call this anyway because it checks
                // whether a gesture is available
                // Seems that this also fixes a bug after 1st init
                handleGesture();
            }
            */
            if (status & 0x10) // AINT from statusReg
            {
                ESP_LOGD(TAG, "ALS interrupt");
                handleALS();
                delay(50);
                yield();
                delay(50);
                apds.clearAlsClearChannelInt();
            }
            if (status & 0x20) // PINT from statusReg
            {
                ESP_LOGD(TAG, "Proximity interrupt");
                delay(50);
                handleProximity();
                yield();
                delay(50);
                apds.clearProximityInt();
            }
        }
        else
        {
            ESP_LOGE(TAG, "Interrupt ERROR");
        }
        apds.clearAmbientLightInt(); // incl. "All non-gesture interrupts clear"

        attachInterrupt(digitalPinToInterrupt(APDS9960_INT), interruptRoutine, FALLING);
        apds.restoreInterrupts();
    }
    else
    {
        ESP_LOGV_N(TAG, ".");
        delay(50);
        if (pollEnabled)
        {
            uint8_t status = apds.getStatusRegister();
            if ((status & 0x34) && (isr_flag == 0)) //  statusReg set but no int received
            {
                ESP_LOGD(TAG, "POLL -----> interrupt register 0x%x", status);
                isr_flag = 1; // "emulate" interrupt
            }
        }
    }
}


//################################################################
static void handleGesture()
{
    if (apds.isGestureAvailable())
    {
        ESP_LOGD(TAG, "GestureAvailable");
        int gesture = apds.readGesture();
        switch (gesture)
        {
        case DIR_UP:
            ESP_LOGD(TAG, "UP");
            Gesture_output += "UP ";
            break;
        case DIR_DOWN:
            Gesture_output += "DOWN ";
            ESP_LOGD(TAG, "DOWN");
            break;
        case DIR_LEFT:
            Gesture_output += "LEFT ";
            ESP_LOGD(TAG, "LEFT");
            break;
        case DIR_RIGHT:
            Gesture_output += "RIGHT ";
            ESP_LOGD(TAG, "RIGHT");
            break;
        case DIR_NEAR:
            Gesture_output += "NEAR ";
            ESP_LOGD(TAG, "NEAR");
            break;
        case DIR_FAR:
            Gesture_output += "FAR ";
            ESP_LOGD(TAG, "FAR");
            break;
        default:
            Gesture_output += "NONE ";
            ESP_LOGD(TAG, "NONE %d", gesture);
        }
    }
    else
        ESP_LOGD(TAG, "NO GestureAvailable");
}

static void handleALS()
{
    uint16_t level;
    apds.readAmbientLight(level);
    ESP_LOGD(TAG, "ALS level=%d", level);
    ALS_output = String("ALS level=");
    ALS_output += String(level);

    apds.readRedLight(level);
    ESP_LOGD(TAG, "ALS RED level=%d", level);
    ALS_output += String(" | RED level=");
    ALS_output += String(level);
    apds.readGreenLight(level);
    ESP_LOGD(TAG, "ALS GREEN level=%d", level);
    ALS_output += String(" | GREEN level=");
    ALS_output += String(level);
    apds.readBlueLight(level);
    ESP_LOGD(TAG, "ALS BLUE level=%d", level);
    ALS_output += String(" | BLUE level=");
    ALS_output += String(level);
}

static void handleProximity()
{
    uint8_t level;
    apds.readProximity(level);
    ESP_LOGD(TAG, "Proximity level=%d", level);
    Proxi_output = "Proximity level=";
    Proxi_output += String(level);
}

// ####################################################
// ####################################################
// ####################################################
// ####################################################
void APDSIF_provideSensorSetupData(DynamicJsonDocument &doc)
{
    // JsonObject alsDoc = doc.createNestedObject(("ALS"));
    uint8_t enabled;
    uint16_t threshold;

    apds.getMode(AMBIENT_LIGHT, enabled);
    doc[F("enableLightSensor")] = enabled ? 1 : 0;
    doc[F("enableLightSensorLL")] = enabled ? 1 : 0;
    doc[F("enableLightSensorInterrupts")] = apds.getAmbientLightIntEnable();
    doc[F("enableLightSensorInterruptsLL")] = apds.getAmbientLightIntEnable();
    doc[F("lightSensorGain")] = apds.getAmbientLightGain();
    apds.getLightIntLowThreshold(threshold);
    doc[F("alsInterruptLowThreshold")] = threshold;
    apds.getLightIntHighThreshold(threshold);
    doc[F("alsInterruptHighThreshold")] = threshold;

    apds.getMode(PROXIMITY, enabled);
    doc[F("enableProximitySensor")] = enabled ? 1 : 0;
    doc[F("enableProximitySensorLL")] = enabled ? 1 : 0;
    doc[F("enableProximitySensorInterrupts")] = apds.getProximityIntEnable();
    doc[F("enableProximitySensorInterruptsLL")] = apds.getProximityIntEnable();
    doc[F("proximitySensorGain")] = apds.getProximityGain();
    doc[F("ledDrive")] = apds.getLEDDrive();
    doc[F("pPulseL")] = 2;  // TODO
    doc[F("pPulseC")] = 10; // TODO
    apds.getProximityIntLowThreshold(enabled);
    doc[F("pInterruptLowThreshold")] = enabled;
    apds.getProximityIntHighThreshold(enabled);
    doc[F("pInterruptHighThreshold")] = enabled;

    apds.getMode(GESTURE, enabled);
    doc[F("enableGestureSensor")] = enabled ? 1 : 0;
    doc[F("enableGestureSensorLL")] = enabled ? 1 : 0;
    doc[F("enableGestureSensorInterrupts")] = apds.getGestureIntEnable();
    doc[F("gestureSensorGain")] = apds.getGestureGain();
    doc[F("ledBoost")] = apds.getLEDBoost();
    doc[F("gestureWaitTime")] = apds.getGestureWaitTime();
    doc[F("gPulseL")] = 2;  // TODO
    doc[F("gPulseC")] = 10; // TODO
    doc[F("gEnterThreshold")] = apds.getGestureEnterThresh();
    doc[F("gExitThreshold")] = apds.getGestureExitThresh();

    apds.getMode(POWER, enabled);
    doc[F("enablePower")] = enabled ? 1 : 0;
    apds.getMode(WAIT, enabled);
    doc[F("enableWaitTimer")] = enabled ? 1 : 0;
    enabled = apds.getGestureMode();
    doc[F("enableGestureMode")] = enabled ? 1 : 0;
}

//
void APDSIF_receiveSensorSetupData(DynamicJsonDocument &doc)
{
    ESP_LOGD(TAG, ("APDSIF_receiveSensorSetupData"));

    uint8_t enabeALS = doc[F("enableLightSensor")];
    uint8_t enableProximity = doc[F("enableProximitySensor")];
    uint8_t enableGesture = doc[F("enableGestureSensor")];

    uint8_t enableALS_Isr = doc[F("enableLightSensorInterrupts")];
    uint8_t enableProximity_Isr = doc[F("enableProximitySensorInterrupts")];
    uint8_t enableGesture_Isr = doc[F("enableGestureSensorInterrupts")];

    //

    if (enabeALS)
    {
        ESP_LOGD(TAG, "ALS enabled");
        apds.enableLightSensor(enableALS_Isr);
        uint16_t threshold;
        threshold = doc[F("alsInterruptLowThreshold")];
        apds.setLightIntLowThreshold(threshold);
        threshold = doc[F("alsInterruptHighThreshold")];
        apds.setLightIntHighThreshold(threshold);
        apds.setAmbientLightGain(doc[F("lightSensorGain")]);
    }
    else
    {
        apds.disableLightSensor();
    }

    //

    if (enableProximity)
    {
        ESP_LOGD(TAG, "PROX enabled");

        apds.enableProximitySensor(enableProximity_Isr);

        apds.setProximityGain(doc[F("proximitySensorGain")]);
        apds.setLEDDrive(doc[F("ledDrive")]);
        uint8_t pPulseL = doc[F("pPulseL")]; // TODO
        uint8_t pPulseC = doc[F("pPulseC")]; // TODO
        apds.setProximityIntLowThreshold(doc[F("pInterruptLowThreshold")]);
        apds.setProximityIntHighThreshold(doc[F("pInterruptHighThreshold")]);
    }
    else
    {
        apds.disableProximitySensor();
    }

    //

    if (enableGesture)
    {
        ESP_LOGD(TAG, "GESTURE enabled");
        apds.enableGestureSensor(enableGesture_Isr);

        apds.setGestureGain(doc[F("gestureSensorGain")]);
        apds.setLEDBoost(doc[F("ledBoost")]);
        uint8_t gPulseL = doc[F("gPulseL")]; // TODO
        uint8_t gPulseC = doc[F("gPulseC")]; // TODO
        apds.setGestureWaitTime(doc[F("gestureWaitTime")]);
        apds.setGestureEnterThresh(doc[F("gEnterThreshold")]);
        apds.setGestureExitThresh(doc[F("gExitThreshold")]);
        apds.setGestureMode(doc[F("enableGestureMode")]);
    }
    else
    {
        apds.disableGestureSensor();
    }
    ALS_output.clear();     // reset
    Proxi_output.clear();   // reset
    Gesture_output.clear(); // reset
}

void APDSIF_receiveSensorLLSetupData(DynamicJsonDocument &doc)
{
    ESP_LOGD(TAG, ("APDSIF_receiveSensorLLSetupData"));
    apds.setMode(POWER, doc[F("enablePower")]);
    apds.setMode(AMBIENT_LIGHT, doc[F("enableLightSensorLL")]);
    apds.setMode(PROXIMITY, doc[F("enableProximitySensorLL")]);
    apds.setMode(WAIT, doc[F("enableWaitTimer")]);
    apds.setMode(PROXIMITY_INT, doc[F("enableProximitySensorInterruptsLL")]);
    apds.setMode(AMBIENT_LIGHT_INT, doc[F("enableLightSensorInterruptsLL")]);
    apds.setMode(GESTURE, doc[F("enableGestureSensorLL")]);
    ALS_output.clear();     // reset
    Proxi_output.clear();   // reset
    Gesture_output.clear(); // reset
}

void APDSIF_provideSensorOutputData(DynamicJsonDocument &doc)
{
    doc[F("ALS_output")] = ALS_output;
    doc[F("Proximity_output")] = Proxi_output;
    doc[F("Gesture_output")] = Gesture_output;
    doc[F("NumInterrupts")] = numInterrupts;
    doc[F("NumPolls")] = numPolls;
    numInterrupts = 0; // reset
    numPolls = 0;      //  reset
}