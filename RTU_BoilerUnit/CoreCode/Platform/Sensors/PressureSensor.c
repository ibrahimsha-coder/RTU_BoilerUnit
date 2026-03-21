#include "PressureSensor.h"
#include "DebugMessage.h"
#include <stdlib.h>

static const char *TAG = "PRESSURE_SENSOR";

static float GetPressureSensorValue(void);

static float currentPressure = 50.0f;

static PressureSensorHandler pressureSensor =
{
    .GetPressureSensorValue = GetPressureSensorValue
};

PressureSensorHandler *CreatePressureSensor(void)
{
    return &pressureSensor;
}

static float GetPressureSensorValue(void)
{
    currentPressure += (rand() % 5) - 2;
    
    if (currentPressure < 50.0f)
    {
        currentPressure = 50.0f;
    }

    if (currentPressure > 300.0f)
    {
        currentPressure = 300.0f;
    }

    float pressure_psi = currentPressure * 0.145038f;

    // ESP_LOGI(TAG, "Pressure: %.2f PSI", pressure_psi);

    // ESP_LOGI(TAG, "Pressure: %.2f kPa", currentPressure);

    return currentPressure;
}