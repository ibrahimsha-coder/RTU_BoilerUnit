#include "TempratureSensor.h"
#include "DebugMessage.h"
#include <stdlib.h>

static const char *TAG = "TEMP_SENSOR";

static float GetTempratureSensorValue(void);

static float currentTemp = 25.0f;
static int direction = 1;

static TempratureSensorHandler tempratureSensor =
{
    .GetTempratureSensorValue = GetTempratureSensorValue
};

TempratureSensorHandler *CreateTempratureSensor(void)
{
    return &tempratureSensor;
}

static float GetTempratureSensorValue(void)
{
    currentTemp += direction * (rand() % 3);

    if (currentTemp >= 110.0f)
    {
        currentTemp = 110.0f;
        direction = -1;  
    }
    else if (currentTemp <= 25.0f)
    {
        currentTemp = 25.0f;
        direction = 1;
    }

    // ESP_LOGI(TAG, "Temperature: %.2f C", currentTemp);

    return currentTemp;
}