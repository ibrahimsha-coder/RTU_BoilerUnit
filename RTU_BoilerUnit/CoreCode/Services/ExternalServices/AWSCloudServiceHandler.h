#ifndef AWS_CLOUD_SERVICE_HANDLER_H
#define AWS_CLOUD_SERVICE_HANDLER_H

#include <stdbool.h>

typedef struct AWSCloudServiceHandler
{
    bool (*Init)(void);
    bool (*SendData)(float temprature, float pressure, int levelLow, int levelHigh, int heater, int pump, int state, int error);
    bool (*ReadCommand)(bool *start);
} AWSCloudServiceHandler;

#endif