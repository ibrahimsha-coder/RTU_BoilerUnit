#ifndef BOILER_UNIT_HANDLER_H
#define BOILER_UNIT_HANDLER_H

#include <stdbool.h>
#include <stddef.h> 

#include "LevelSensorDefinitions.h"
#include "BoilerDefinition.h"

typedef struct BoilerUnitHandler
{
    BoilerStatus (*PumpWaterBoilerUnit)(void);
    BoilerStatus (*StopWaterSupplyBoilerUnit)(void);
    float (*GetTemperature)(void);
    float (*GetPressure)(void);
    LevelState (*GetLevel)(void);
    BoilerStatus (*StartHeating)(void);
    BoilerStatus (*StopHeating)(void);
} BoilerUnitHandler;

#endif//BOILER_UNIT_HANDLER_H