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
    static float currentPressure = 100.0f; // start ~14.5 PSI

    // --- simulate heating effect ---
    if (currentPressure < 120.0f)
    {
        currentPressure += 1.5f;  // increase pressure (heater ON effect)
    }
    else
    {
        currentPressure -= 1.0f;  // natural cooling
    }

    // --- clamp realistic range ---
    if (currentPressure < 90.0f)
    {
        currentPressure = 90.0f;
    }

    if (currentPressure > 130.0f)
    {
        currentPressure = 130.0f;
    }

    float pressure_psi = currentPressure * 0.145038f;

    return pressure_psi;
}