#include "TempratureSensor.h"
#include "DebugMessage.h"
#include <stdlib.h>

static const char *TAG = "TEMP_SENSOR";

static float GetTempratureSensorValue(void);

/* ================= STATE ================= */

static float currentTemp = 25.0f;   // initial temp
static int direction = 1;           // 1 = heating, -1 = cooling

/* ================= HANDLER ================= */

static TempratureSensorHandler tempratureSensor =
{
    .GetTempratureSensorValue = GetTempratureSensorValue
};

TempratureSensorHandler *CreateTempratureSensor(void)
{
    return &tempratureSensor;
}

/* ================= SIMULATION ================= */

static float GetTempratureSensorValue(void)
{
    /* Simulate gradual change */
    currentTemp += direction * (rand() % 3);  // slow variation

    /* Clamp + reverse direction */
    if (currentTemp >= 110.0f)
    {
        currentTemp = 110.0f;
        direction = -1;  // start cooling
    }
    else if (currentTemp <= 25.0f)
    {
        currentTemp = 25.0f;
        direction = 1;   // start heating
    }

    ESP_LOGI(TAG, "Temperature: %.2f C", currentTemp);

    return currentTemp;
}