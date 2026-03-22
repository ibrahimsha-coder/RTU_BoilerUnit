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
    static int highHoldCounter = 0;

    // --- FAULT STATE LOCK ---
    if (internal_level < 0)
    {
        return;
    }

    // --- OPTIONAL FAULT (DISABLED NOW) ---
    /*
    static int faultCounter = 0;
    faultCounter++;

    if (faultCounter >= 25)
    {
        internal_level = LEVEL_INVALID;
        faultCounter = 0;
        return;
    }
    */

    // --- HOLD HIGH LEVEL FOR 100 CYCLES ---
    if (internal_level == LEVEL_HIGH)
    {
        highHoldCounter++;

        if (highHoldCounter < 100)
        {
            return;  // maintain HIGH
        }

        highHoldCounter = 0;
    }

    int change = rand() % 3;

    if (internal_level == LEVEL_EMPTY)
    {
        if (change == 0)
        {
            internal_level = LEVEL_LOW;
        }
    }
    else if (internal_level == LEVEL_LOW)
    {
        if (change == 0)
        {
            internal_level = LEVEL_HIGH;
        }
        else if (change == 1)
        {
            internal_level = LEVEL_EMPTY;
        }
    }
    else if (internal_level == LEVEL_HIGH)
    {
        if (change == 0)
        {
            internal_level = LEVEL_LOW;
        }
    }
}

static LevelState GetLevelSensorValue(void)
{
    SimulateLevel();

    LevelState state = LEVEL_INVALID;

    if (internal_level == LEVEL_EMPTY)
    {
        state = LEVEL_EMPTY;
    }
    else if (internal_level == LEVEL_LOW)
    {
        state = LEVEL_LOW;
    }
    else if (internal_level == LEVEL_HIGH)
    {
        state = LEVEL_HIGH;
    }
    else
    {
        state = LEVEL_INVALID;
    }

    return state;
}