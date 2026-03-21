#ifndef PRESSURE_SENSOR_HANDLER_H
#define PRESSURE_SENSOR_HANDLER_H

#include <stdbool.h>

typedef struct PressureSensorHandler
{
    float (*GetPressureSensorValue)(void);
} PressureSensorHandler;

#endif//PRESSURE_SENSOR_HANDLER_H