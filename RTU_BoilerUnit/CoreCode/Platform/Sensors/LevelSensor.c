//
// © 2025 BoatLoadMinds PVT LMT. All Rights Reserved.
//

#include "LevelSensor.h"
#include "DebugMessage.h"
#include <stdlib.h>

static const char *TAG = "LEVEL_SENSOR";

static int internal_level = 0;  

static LevelState GetLevelSensorValue(void);

static LevelSensorHandler levelSensorHandler =
{
    GetLevelSensorValue
};

LevelSensorHandler *CreateLevelSensor(void)
{
    return &levelSensorHandler;
}

static void SimulateLevel(void)
{
    int change = rand() % 4;

    if (change == 0 && internal_level < 2)
    {
        internal_level++;   // filling
    }
    else if (change == 1 && internal_level > 0)
    {
        internal_level--;   // draining
    }
    else if (change == 3)
    {
        internal_level = -1; // fault condition
    }
    // else hold state
}

static LevelState GetLevelSensorValue(void)
{
    SimulateLevel();

    LevelState state = LEVEL_INVALID;

    if (internal_level == 0)
    {
        state = LEVEL_EMPTY;
    }
    else if (internal_level == 1)
    {
        state = LEVEL_LOW;
    }
    else if (internal_level == 2)
    {
        state = LEVEL_HIGH;
    }
    else
    {
        state = LEVEL_INVALID;
    }

    // ESP_LOGI(TAG, "Level State: %d", state);

    return state;
}