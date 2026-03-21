#ifndef BOILER_H
#define BOILER_H

#include "TempratureSensorHandler.h"
#include "PressureSensorHandler.h"
#include "LevelSensorHandler.h"
#include "PumpHandler.h"
#include "HeaterHandler.h"
#include "BoilerUnitHandler.h"

BoilerUnitHandler *CreateBoilerUnit(TempratureSensorHandler *tempratureSensorObject,
                                         PressureSensorHandler *pressureSensorObject,
                                         LevelSensorHandler *levelSensorObject,
                                         PumpHandler *pumpObject,
                                         HeaterHandler *heaterObject);

#endif// BOILER_H