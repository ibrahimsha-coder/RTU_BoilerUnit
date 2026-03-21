#ifndef LEVEL_SENSOR_HANDLER_H
#define LEVEL_SENSOR_HANDLER_H

#include <stdbool.h>

#include "LevelSensorDefinitions.h"

typedef struct LevelSensorHandler
{
    LevelState (*GetLevelSensorValue)(void);
} LevelSensorHandler;

#endif//LEVEL_SENSOR_HANDLER_H