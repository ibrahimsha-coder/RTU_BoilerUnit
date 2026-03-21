#ifndef RTU_PROCESS_H
#define RTU_PROCESS_H

#include <stdbool.h>
#include <stdint.h>

#include "WifiHandler.h"
#include "CloudServiceHandler.h"
#include "BoilerUnitHandler.h"
#include "RTCProcessHandler.h"

#define OVER_PRESSURE 255
#define INVALID_PRESSURE 0

#define MINIMUM_THRESHOLD_TEMPRATURE 33
#define MAXIMUM_THRESHOLD_TEMPRATURE 120

#define MINIMUM_THRESHOLD_PRESSURE 14
#define MAXIMUM_THRESHOLD_PRESSURE 18

#define KPA_TO_PSI 0.145038f

#define ERROR_COUNT 3

#define FLASH_NAMESPACE      "rtu_store"
#define FLASH_INDEX_KEY      "index"
#define FLASH_RECORD_PREFIX  "rec_"
#define FLASH_MAX_RECORDS    50

RTCProcessHandler *CreateRTUProcess(WifiHandler *wifiHandlerObject, CloudServiceHandler *cloudHandlerObject, BoilerUnitHandler *boilerObject);

typedef enum BoilerError
{
    ERROR_NONE,
    ERROR_LEVEL_SENSOR,
    ERROR_PRESSURE_SENSOR,
    ERROR_TEMPERATURE_SENSOR,
    ERROR_DRY_RUN
} BoilerError;

typedef enum BoilerState
{
    BOILER_IDLE,
    BOILER_FILLING,
    BOILER_HEATING,
    BOILER_READY,
    BOILER_ERROR
} BoilerState;

typedef struct BoilerSensors
{
    float temperature;
    float pressure;
    float level;
    bool levelLow;
    bool levelHigh;
} BoilerSensors;

typedef struct FlashRecord
{
    float temperature;
    float pressure;
    bool levelLow;
    bool levelHigh;
    int heater;
    int pump;
    int state;
    int error;
    uint32_t timestamp;
} FlashRecord;

#endif // RTU_PROCESS_H