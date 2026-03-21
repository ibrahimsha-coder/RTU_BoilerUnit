#ifndef BOILER_UNIT_HANDLER_H
#define BOILER_UNIT_HANDLER_H

#include <stdbool.h>
#include <stddef.h> 

typedef struct BoilerUnitHandler
{
    bool (*PumpWaterBoilerUnit)(void);
    bool (*StopWaterSupplyBoilerUnit)(void);
    float (*GetTemperature)(void);
    float (*GetPressure)(void);
    float (*GetLevel)(void);
    bool (*StartHeating)(void);
    bool (*StopHeating)(void);
} BoilerUnitHandler;

#endif//BOILER_UNIT_HANDLER_H