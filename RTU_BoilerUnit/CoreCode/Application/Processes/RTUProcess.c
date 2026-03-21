//
// © 2025 BoatLoadMinds PVT LMT. All Rights Reserved.
//

#include "OS.h"
#include "esp_timer.h"

#include "RTUProcess.h"
#include "DebugMessage.h"

static const char *TAG = "RTU";

static RTCProcessHandler *rtu = NULL;
static WifiHandler *wifiHandler = NULL;
static CloudServiceHandler *cloudHandler = NULL;
static BoilerUnitHandler *boiler = NULL;

static int levelErrorCount = 0;
static int tempErrorCount  = 0;

static int currentError = ERROR_NONE;
/* ===================== */

/* ================= PROTOTYPES ================= */

static bool Init(void);
static void RTUApplication_task(void *pvParameters);

static BoilerState HandleBoiler(BoilerState state, BoilerSensors *s, int *errorCode);
static const char* GetErrorString(int error);

static RTCProcessHandler rtuHandler =
{
    .Init = Init
};

RTCProcessHandler *CreateRTUProcess(WifiHandler *wifiHandlerObject, CloudServiceHandler *cloudHandlerObject, BoilerUnitHandler *boilerObject)
{
    if (wifiHandlerObject && cloudHandlerObject && boilerObject)
    {
        wifiHandler = wifiHandlerObject;
        cloudHandler = cloudHandlerObject;
        boiler = boilerObject;

        xTaskCreatePinnedToCore(RTUApplication_task, "RTUApplication_task", 8000, NULL, 5, NULL, 1);
    }

    return &rtuHandler;
}

static bool Init(void)
{
    ESP_LOGI(TAG, "RTU Init");
    return true;
}

static BoilerState HandleBoiler(BoilerState state, BoilerSensors *sensor, int *errorCode)
{
    /* ===== FIX ADDED ===== */
    float pressure_psi = sensor->pressure * KPA_TO_PSI;
    /* ===================== */

    if (sensor->level < 0)
    {
        levelErrorCount++;

        if (levelErrorCount >= ERROR_COUNT)
        {
            currentError = ERROR_LEVEL_SENSOR;
            return BOILER_ERROR;
        }
    }
    else
    {
        levelErrorCount = 0;
    }

    if (sensor->pressure == INVALID_PRESSURE || sensor->pressure == OVER_PRESSURE)
    {
        currentError = ERROR_PRESSURE_SENSOR;
        return BOILER_ERROR;
    }

    if (sensor->temperature > MAXIMUM_THRESHOLD_TEMPRATURE)
    {
        tempErrorCount++;

        if (tempErrorCount >= ERROR_COUNT)
        {
            currentError = ERROR_TEMPERATURE_SENSOR;
            return BOILER_ERROR;
        }
    }
    else
    {
        tempErrorCount = 0;
    }

    if (state == BOILER_IDLE)
    {
        boiler->StopHeating();
        boiler->StopWaterSupplyBoilerUnit();

        /* ===== FIX ADDED ===== */
        if (currentError != ERROR_NONE)
        {
            ESP_LOGI(TAG, "ERROR CLEARED");
            currentError = ERROR_NONE;
        }
        /* ===================== */

        if (sensor->levelLow)
        {
            ESP_LOGW(TAG, "LOW LEVEL → FILLING");
            return BOILER_FILLING;
        }

        return BOILER_HEATING;
    }

    else if (state == BOILER_FILLING)
    {
        boiler->PumpWaterBoilerUnit();
        boiler->StopHeating();

        if (sensor->levelHigh)
        {
            ESP_LOGI(TAG, "TANK FULL → HEATING");
            return BOILER_HEATING;
        }
    }

    else if (state == BOILER_HEATING)
    {
        if (!sensor->levelLow)
        {
            currentError = ERROR_DRY_RUN;
            return BOILER_ERROR;
        }

        boiler->StopWaterSupplyBoilerUnit();

        /* ===== FIX UPDATED ===== */
        if (pressure_psi < MINIMUM_THRESHOLD_PRESSURE)
        {
            boiler->StartHeating();
        }
        else if (pressure_psi >= MAXIMUM_THRESHOLD_PRESSURE)
        {
            boiler->StopHeating();
        }
        /* ======================= */

        if (sensor->pressure > OVER_PRESSURE)
        {
            currentError = ERROR_PRESSURE_SENSOR;
            return BOILER_ERROR;
        }
    }

    else if (state == BOILER_READY)
    {
        boiler->StopHeating();

        if (pressure_psi < MINIMUM_THRESHOLD_PRESSURE)
        {
            return BOILER_HEATING;
        }
    }

    else if (state == BOILER_ERROR)
    {
        boiler->StopHeating();
        boiler->StopWaterSupplyBoilerUnit();

        ESP_LOGE(TAG, "PROCESS STOPPED DUE TO ERROR");

        return BOILER_ERROR;
    }

    return state;
}

static const char* GetErrorString(int error)
{
    if (error == ERROR_NONE)
    {
        return "NO_ERROR";
    }
    else if (error == ERROR_LEVEL_SENSOR)
    {
        return "LEVEL_SENSOR_ERROR";
    }
    else if (error == ERROR_PRESSURE_SENSOR)
    {
        return "PRESSURE_SENSOR_ERROR";
    }
    else if (error == ERROR_TEMPERATURE_SENSOR)
    {
        return "TEMPERATURE_SENSOR_ERROR";
    }
    else if (error == ERROR_DRY_RUN)
    {
        return "DRY_RUN_ERROR";
    }
    else
    {
        return "UNKNOWN_ERROR";
    }
}

void RTUApplication_task(void *pvParameters)
{
    srand(esp_timer_get_time());

    BoilerSensors sensors = {0};
    BoilerState state = BOILER_IDLE;

    bool start = false;

    vTaskDelay(pdMS_TO_TICKS(5000));

    while (1)
    {
        if (wifiHandler && wifiHandler->GetStatus() == WIFI_STATUS_CONNECTED)
        {
            cloudHandler->ReadCommand(&start);
            if (start)
            {
                sensors.temperature = boiler->GetTemperature();
                sensors.pressure    = boiler->GetPressure();
                sensors.level       = boiler->GetLevel();

                /* ===== FIX UPDATED ===== */
                sensors.levelLow  = (sensors.level >= 1);
                sensors.levelHigh = (sensors.level == 2);
                /* ======================= */

                /* ===== FIX UPDATED ===== */
                state = HandleBoiler(state, &sensors, NULL);

                int error = currentError;
                /* ======================= */

                int heater = (state == BOILER_HEATING) ? 1 : 0;
                int pump   = (state == BOILER_FILLING) ? 1 : 0;

                ESP_LOGI(TAG,"Temp: %.2f | Press: %.2f | Level: %.2f | Low:%d High:%d | Heater:%d Pump:%d | State:%d | Error:%d (%s)",
                         sensors.temperature,
                         sensors.pressure,
                         sensors.level,
                         sensors.levelLow,
                         sensors.levelHigh,
                         heater,
                         pump,
                         state,
                         error,
                         GetErrorString(error));

                cloudHandler->SendData(sensors.temperature, sensors.pressure, sensors.levelLow, sensors.levelHigh, heater, pump, state, error);
            }
        }
        else
        {
            ESP_LOGW(TAG, "WiFi Not Connected");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}