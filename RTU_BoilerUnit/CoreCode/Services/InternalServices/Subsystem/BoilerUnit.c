#include "BoilerUnit.h"
#include "DebugMessage.h"


static const char *TAG = "BOILER";

static TempratureSensorHandler *tempratureSensor = NULL;
static PressureSensorHandler *pressureSensor = NULL;
static LevelSensorHandler *levelSensor = NULL;
static PumpHandler *pump = NULL;
static HeaterHandler *heater = NULL;

static bool PumpWaterBoilerUnit(void);
static bool StopWaterSupplyBoilerUnit(void);
static float GetTemperature(void);
static float GetPressure(void);
static float GetLevel(void);
static bool StartHeating(void);
static bool StopHeating(void);

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

static bool PumpWaterBoilerUnit(void)
{
    if (levelSensor == NULL || pump == NULL)
    {
        return false;
    }

      LevelState level = levelSensor->GetLevelSensorValue();

    if (level == LEVEL_HIGH)
    {
        ESP_LOGW(TAG, "Tank Full → Stopping Pump");
        pump->PumpOff();
        return false;
    }

    ESP_LOGI(TAG, "Filling Boiler... Level: %d", level);

    if (pump->PumpOn() == true)
    {
        return true;
    }

    ESP_LOGE(TAG, "Pump Start Failed");
    return false;
}

static bool StopWaterSupplyBoilerUnit(void)
{
    if (pump == NULL)
    {
        return false;
    }

    if (pump->PumpOff() == true)
    {
        ESP_LOGI(TAG, "Pump Stopped");
        return true;
    }

    ESP_LOGE(TAG, "Pump Stop Failed");
    return false;
}

static float GetTemperature(void)
{
    return tempratureSensor->GetTempratureSensorValue();
}

static float GetPressure(void)
{
    return pressureSensor->GetPressureSensorValue();
}

static float GetLevel(void)
{
    return levelSensor->GetLevelSensorValue();
}

static bool StartHeating(void)
{
    if (heater == NULL)
    {
        return false;
    }

    float temprature = GetPressure();
    float pressure = GetPressure();

    if (temprature > 120.0f)
    {
        ESP_LOGE(TAG, "Over Temperature! Heater OFF");
        heater->HeaterOff();
        return false;
    }

    ESP_LOGI(TAG, "Heater ON");

    return heater->HeaterOn();
}

static bool StopHeating(void)
{
    if (heater == NULL)
    {
        return false;
    }

    ESP_LOGI(TAG, "Heater OFF");

    return heater->HeaterOff();
}