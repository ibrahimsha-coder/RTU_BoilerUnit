#ifndef LEVEL_SENSOR_HANDLER_H
#define LEVEL_SENSOR_HANDLER_H

#include <stdbool.h>

typedef enum LevelState
{
    LEVEL_EMPTY,
    LEVEL_LOW,
    LEVEL_HIGH,
    LEVEL_INVALID
} LevelState;

typedef struct LevelSensorHandler
{
    LevelState (*GetLevelSensorValue)(void);
} LevelSensorHandler;

#endif//LEVEL_SENSOR_HANDLER_H