#include "PressureSensor.h"
#include "DebugMessage.h"
#include <stdlib.h>

static const char *TAG = "PRESSURE_SENSOR";

static float GetPressureSensorValue(void);

/* ================= STATE ================= */

static float currentPressure = 50.0f;

/* ================= HANDLER ================= */

static PressureSensorHandler pressureSensor =
{
    .GetPressureSensorValue = GetPressureSensorValue
};

PressureSensorHandler *CreatePressureSensor(void)
{
    return &pressureSensor;
}

/* ================= SIMULATION ================= */

static float GetPressureSensorValue(void)
{
    /* Simulate pressure change */
    currentPressure += (rand() % 5) - 2;   // small fluctuation

    /* Clamp limits */
    if (currentPressure < 50.0f)
        currentPressure = 50.0f;

    if (currentPressure > 300.0f)
        currentPressure = 300.0f;

    ESP_LOGI(TAG, "Pressure: %.2f kPa", currentPressure);

    return currentPressure;
}