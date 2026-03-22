#include "BoilerUnit.h"
#include "DebugMessage.h"


static const char *TAG = "BOILER";

static TempratureSensorHandler *tempratureSensor = NULL;
static PressureSensorHandler *pressureSensor = NULL;
static LevelSensorHandler *levelSensor = NULL;
static PumpHandler *pump = NULL;
static HeaterHandler *heater = NULL;

static BoilerStatus PumpWaterBoilerUnit(void);
static BoilerStatus StopWaterSupplyBoilerUnit(void);
static float GetTemperature(void);
static float GetPressure(void);
static LevelState GetLevel(void);
static BoilerStatus StartHeating(void);
static BoilerStatus StopHeating(void);

static BoilerUnitHandler boilerUnitHandler =
{
    PumpWaterBoilerUnit,
    StopWaterSupplyBoilerUnit,
    GetTemperature,
    GetPressure,
    GetLevel,
    StartHeating,
    StopHeating
};

BoilerUnitHandler *CreateBoilerUnit(TempratureSensorHandler *tempratureSensorObject, PressureSensorHandler *pressureSensorObject, LevelSensorHandler *levelSensorObject, PumpHandler *pumpObject, HeaterHandler *heaterObject)
{
    if (tempratureSensorObject != NULL && pressureSensorObject != NULL && levelSensorObject != NULL && pumpObject != NULL && heaterObject != NULL)
    {
        tempratureSensor = tempratureSensorObject;
        pressureSensor = pressureSensorObject;
        levelSensor = levelSensorObject;
        pump = pumpObject;
        heater = heaterObject;        
    }

    return &boilerUnitHandler;
}

static BoilerStatus PumpWaterBoilerUnit(void)
{
    if (levelSensor == NULL || pump == NULL)
    {
        return BOILER_ERROR_NULL;
    }

    LevelState level = levelSensor->GetLevelSensorValue();

    if (level == LEVEL_HIGH)
    {
        ESP_LOGW(TAG, "Tank Full Stopping Pump");
        pump->PumpOff();
        return BOILER_OK;
    }

    if (level == LEVEL_EMPTY)
    {
        ESP_LOGW(TAG, "Tank Level is empty");
        return pump->PumpOn() ? BOILER_OK : BOILER_ERROR_NULL;
    }

    if (level == LEVEL_INVALID)
    {
        ESP_LOGW(TAG, "Tank Level is empty");
        pump->PumpOff();

        if (heater != NULL)
        {
            heater->HeaterOff();
        }

        return BOILER_ERROR_INVALID_LEVEL;
    }

    ESP_LOGI(TAG, "Filling Boiler... Level: %d", level);

     return pump->PumpOn() ? BOILER_OK : BOILER_ERROR_NULL;
}

static BoilerStatus StopWaterSupplyBoilerUnit(void)
{
    if (pump == NULL)
    {
        return BOILER_ERROR_NULL;
    }

    ESP_LOGI(TAG, "Filling Process Stopped");

    return pump->PumpOff() ? BOILER_OK : BOILER_ERROR_NULL;
}

static float GetTemperature(void)
{
    return tempratureSensor->GetTempratureSensorValue();
}

static float GetPressure(void)
{
    return pressureSensor->GetPressureSensorValue();
}

static LevelState GetLevel(void)
{
    return levelSensor->GetLevelSensorValue();
}

static BoilerStatus StartHeating(void)
{
    if (heater == NULL || levelSensor == NULL)
    {
        return BOILER_ERROR_NULL;
    }

    LevelState level = levelSensor->GetLevelSensorValue();

    if (level == LEVEL_EMPTY || level == LEVEL_INVALID)
    {
        heater->HeaterOff();
        return BOILER_ERROR_DRY_RUN;
    }

    float temprature = GetTemperature();
    float pressure = GetPressure();

    float pressure_psi = pressure * 0.145038f;

    if (temprature > 120.0f)
    {
        heater->HeaterOff();
        ESP_LOGW(TAG, "BOILER_ERROR_OVER_TEMP");
        return BOILER_ERROR_OVER_TEMP;
    }

    if (pressure_psi >= 16.0f)
    {
        ESP_LOGW(TAG, "BOILER_ERROR_HIGH_PRESSURE");
        heater->HeaterOff();
        return BOILER_ERROR_HIGH_PRESSURE;
    }

    if (pressure_psi <= 14.0f)
    {
        return heater->HeaterOn() ? BOILER_OK : BOILER_ERROR_NULL;
    }

    return BOILER_OK;
}

static BoilerStatus StopHeating(void)
{
    if (heater == NULL)
    {
        return false;
    }

    ESP_LOGI(TAG, "Heater OFF");

    return heater->HeaterOff();
}