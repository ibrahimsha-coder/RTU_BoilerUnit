#ifndef TEMPRATURE_SENSOR_HANDLER_H
#define TEMPRATURE_SENSOR_HANDLER_H

#include <stdbool.h>

typedef struct TempratureSensorHandler
{
    float (*GetTempratureSensorValue)(void);
} TempratureSensorHandler;

#endif//TEMPRATURE_SENSOR_HANDLER_H