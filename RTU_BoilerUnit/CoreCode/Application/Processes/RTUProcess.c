
#include "OS.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"

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

static bool Init(void);
static BoilerState HandleBoiler(BoilerState state, BoilerSensors *s, int *errorCode);
static void StoreToFlash(const FlashRecord *rec);
static void SendStoredData(void);
static const char* GetErrorString(int error);
static void RTUApplication_task(void *pvParameters);

static RTCProcessHandler rtuHandler =
{
    Init
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

    if (nvs_flash_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS Init Failed");
        return false;
    }

    return true;
}

static BoilerState HandleBoiler(BoilerState state, BoilerSensors *sensor, int *errorCode)
{
    float pressure_psi = sensor->pressure * KPA_TO_PSI;

    if (sensor->pressure == INVALID_PRESSURE || sensor->pressure == OVER_PRESSURE)
    {
        currentError = ERROR_PRESSURE_SENSOR;
        return BOILER_ERROR;
    }

    if (state == BOILER_IDLE)
    {
        if (sensor->levelLow)
        {
            return BOILER_FILLING;
        }

        return BOILER_HEATING;
    }

    else if (state == BOILER_FILLING)
    {
        boiler->PumpWaterBoilerUnit();

        if (sensor->levelHigh)
        {
            return BOILER_HEATING;
        }
    }

    else if (state == BOILER_HEATING)
    {
        if (pressure_psi < MINIMUM_THRESHOLD_PRESSURE)
        {
            boiler->StartHeating();
        }
        else if (pressure_psi >= MAXIMUM_THRESHOLD_PRESSURE)
        {
            boiler->StopHeating();
        }
    }

    else if (state == BOILER_READY)
    {
        if (pressure_psi < MINIMUM_THRESHOLD_PRESSURE)
        {
            return BOILER_HEATING;
        }
    }

    else if (state == BOILER_ERROR)
    {
        boiler->StopHeating();
    }

    return state;
}

static void StoreToFlash(const FlashRecord *rec)
{
    nvs_handle_t handle;

    if (nvs_open(FLASH_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK)
    {
        return;
    }

    int32_t index = 0;
    nvs_get_i32(handle, FLASH_INDEX_KEY, &index);

    char key[16];
    snprintf(key, sizeof(key), "%s%ld", FLASH_RECORD_PREFIX, index);

    if (nvs_set_blob(handle, key, rec, sizeof(FlashRecord)) == ESP_OK)
    {
        ESP_LOGI(TAG, "Stored in Flash [%ld]", index);
    }

    index = (index + 1) % FLASH_MAX_RECORDS;
    nvs_set_i32(handle, FLASH_INDEX_KEY, index);

    nvs_commit(handle);
    nvs_close(handle);
}

static void SendStoredData(void)
{
    nvs_handle_t handle;

    if (nvs_open(FLASH_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK)
        return;

    for (int i = 0; i < FLASH_MAX_RECORDS; i++)
    {
        char key[16];
        snprintf(key, sizeof(key), "%s%d", FLASH_RECORD_PREFIX, i);

        FlashRecord rec;
        size_t size = sizeof(rec);

        if (nvs_get_blob(handle, key, &rec, &size) == ESP_OK)
        {
            ESP_LOGI(TAG, "Sending Stored [%d]", i);

            bool success = cloudHandler->SendData(rec.temperature, rec.pressure, rec.levelLow, rec.levelHigh, rec.heater, rec.pump, rec.state, rec.error);

            if (success == true)
            {
                nvs_erase_key(handle, key);
                ESP_LOGI(TAG, "Deleted [%d]", i);
            }
            else
            {
                break;
            }
        }
    }

    nvs_commit(handle);
    nvs_close(handle);
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
    BoilerSensors sensors = {0};
    BoilerState state = BOILER_IDLE;

    bool start = false;

    vTaskDelay(pdMS_TO_TICKS(5000));

    while (1)
    {
        if (wifiHandler && wifiHandler->GetStatus() == WIFI_STATUS_CONNECTED)
        {
            cloudHandler->ReadCommand(&start);
        }

        if (start)
        {
            sensors.temperature = boiler->GetTemperature();
            sensors.pressure    = boiler->GetPressure();
            sensors.level       = boiler->GetLevel();

            sensors.levelLow  = (sensors.level >= 1);
            sensors.levelHigh = (sensors.level == 2);

            state = HandleBoiler(state, &sensors, NULL);

            int error = currentError;

            int heater = (state == BOILER_HEATING);
            int pump   = (state == BOILER_FILLING);

            FlashRecord rec =
            {
                sensors.temperature,
                sensors.pressure,
                sensors.levelLow,
                sensors.levelHigh,
                heater,
                pump,
                state,
                error,
                (uint32_t)(esp_timer_get_time() / 1000)
            };

            bool cloudSendDataFeedBack = false;

            if (wifiHandler && wifiHandler->GetStatus() == WIFI_STATUS_CONNECTED)
            {
                cloudSendDataFeedBack = cloudHandler->SendData(rec.temperature, rec.pressure, rec.levelLow, rec.levelHigh, rec.heater, rec.pump, rec.state, rec.error);
            }

            if (cloudSendDataFeedBack == false)
            {
                ESP_LOGW(TAG, "Cloud Failed → Store in Flash");
                StoreToFlash(&rec);
            }
            else
            {
                ESP_LOGI(TAG, "Cloud OK Send Stored Data");
                SendStoredData();
            }

            ESP_LOGI(TAG,"Temp: %.2f | Press: %.2f | Level: %.2f | State:%d | Error:%s", rec.temperature, rec.pressure, sensors.level, rec.state, GetErrorString(rec.error));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
