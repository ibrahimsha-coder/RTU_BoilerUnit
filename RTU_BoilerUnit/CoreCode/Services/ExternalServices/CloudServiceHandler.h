#ifndef CLOUD_SERVICE_HANDLER_H
#define CLOUD_SERVICE_HANDLER_H

#include <stdbool.h>

typedef struct CloudServiceHandler
{
    bool (*Init)(void);
    bool (*SendData)(float temprature, float pressure, int levelLow, int levelHigh, int heater, int pump, int state, int error);
    bool (*ReadCommand)(bool *start);
} CloudServiceHandler;

#endif// CLOUD_SERVICE_HANDLER_H