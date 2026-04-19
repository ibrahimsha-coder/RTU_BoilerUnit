#include "OS.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "RTUProcess.h"
#include "DebugMessage.h"

static const char *TAG = "RTU";

static RTCProcessHandler *rtu = NULL;
static WifiHandler *wifiHandler = NULL;
static AWSCloudServiceHandler *cloudHandler = NULL;
static BoilerUnitHandler *boiler = NULL;

static int levelErrorCount = 0;
static int tempErrorCount  = 0;

static int currentError = ERROR_NONE;

static bool Init(void);
static BoilerState HandleBoiler(BoilerState state, BoilerSensors *s, int *errorCode);
static void StoreToFlash(const FlashRecord *rec);
static void SendStoredData(void);
static const char* GetErrorString(int error);
static const char* GetErrorString(int error);
static const char* GetStateString(BoilerState state);
static BoilerError MapBoilerStatusToError(BoilerStatus status);
static void RTUApplication_task(void *pvParameters);

static RTCProcessHandler rtuHandler =
{
    Init
};

RTCProcessHandler *CreateRTUProcess(WifiHandler *wifiHandlerObject, AWSCloudServiceHandler *cloudHandlerObject, BoilerUnitHandler *boilerObject)
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
    if (sensor->pressure == 0 || sensor->pressure == 255)
    {
        currentError = ERROR_PRESSURE_SENSOR;
    }

    if (sensor->temperature == 0 || sensor->temperature == 255)
    {
        currentError = ERROR_TEMPERATURE_SENSOR;
    }

    if (sensor->levelHigh && !sensor->levelLow)
    {
        currentError = ERROR_LEVEL_SENSOR;
    }

    // --- FORCE STOP ON ERROR ---
    if (currentError != ERROR_NONE)
    {
        boiler->StopHeating();
        boiler->StopWaterSupplyBoilerUnit();
        return BOILER_ERROR;
    }

    // --- LEVEL CONTROL ---
    if (!sensor->levelHigh) // EMPTY or LOW
    {
        BoilerStatus status = boiler->PumpWaterBoilerUnit();

        if (status != BOILER_OK)
        {
            currentError = MapBoilerStatusToError(status);
            return BOILER_ERROR;
        }

        return BOILER_FILLING;
    }

    // --- HIGH LEVEL ---
    boiler->StopWaterSupplyBoilerUnit();

    float pressure = sensor->pressure;

    // --- READY STATE (NEW FIX) ---
    if (pressure >= MAXIMUM_THRESHOLD_PRESSURE) // ≥ 18 PSI
    {
        boiler->StopHeating();
        return BOILER_READY;
    }

    // --- HEATING ---
    BoilerStatus status = boiler->StartHeating();

    if (status != BOILER_OK)
    {
        currentError = MapBoilerStatusToError(status);
        return BOILER_ERROR;
    }

    return BOILER_HEATING;
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
    snprintf(key, sizeof(key), "%s%d", FLASH_RECORD_PREFIX, (int)index);

    if (nvs_set_blob(handle, key, rec, sizeof(FlashRecord)) == ESP_OK)
    {
        ESP_LOGI(TAG, "Stored in Flash [%d]", (int)index);
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

static const char* GetStateString(BoilerState state)
{
    if (state == BOILER_IDLE) return "IDLE";
    else if (state == BOILER_FILLING) return "FILLING";
    else if (state == BOILER_HEATING) return "HEATING";
    else if (state == BOILER_READY) return "READY";
    else if (state == BOILER_ERROR) return "ERROR";
    else return "UNKNOWN";
}

static BoilerError MapBoilerStatusToError(BoilerStatus status)
{
    if (status == BOILER_ERROR_INVALID_LEVEL) return ERROR_LEVEL_SENSOR;
    else if (status == BOILER_ERROR_DRY_RUN) return ERROR_DRY_RUN;
    else if (status == BOILER_ERROR_OVER_TEMP) return ERROR_TEMPERATURE_SENSOR;
    else if (status == BOILER_ERROR_HIGH_PRESSURE) return ERROR_PRESSURE_SENSOR;
    else return ERROR_NONE;
}

void RTUApplication_task(void *pvParameters)
{
    BoilerSensors sensors = {0};
    BoilerState state = BOILER_IDLE;
    BoilerState prevState = BOILER_IDLE;

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
             currentError = ERROR_NONE;

            // --- RETRY TEMP ---
            for (int i = 0; i < ERROR_COUNT; i++)
            {
                sensors.temperature = boiler->GetTemperature();
                if (sensors.temperature != 0 && sensors.temperature != 255) break;
                vTaskDelay(pdMS_TO_TICKS(50));
                if (i == ERROR_COUNT - 1) currentError = ERROR_TEMPERATURE_SENSOR;
            }

            // --- RETRY PRESSURE ---
            for (int i = 0; i < ERROR_COUNT; i++)
            {
                sensors.pressure = boiler->GetPressure();
                if (sensors.pressure != 0 && sensors.pressure != 255) break;
                vTaskDelay(pdMS_TO_TICKS(50));
                if (i == ERROR_COUNT - 1) currentError = ERROR_PRESSURE_SENSOR;
            }

            // --- RETRY LEVEL ---
            LevelState rawLevel = LEVEL_INVALID;

            for (int i = 0; i < ERROR_COUNT; i++)
            {
                rawLevel = boiler->GetLevel();
                if (rawLevel != LEVEL_INVALID) break;
                vTaskDelay(pdMS_TO_TICKS(50));
                if (i == ERROR_COUNT - 1) currentError = ERROR_LEVEL_SENSOR;
            }

            sensors.level = rawLevel;
            sensors.levelLow  = (rawLevel == LEVEL_LOW || rawLevel == LEVEL_HIGH);
            sensors.levelHigh = (rawLevel == LEVEL_HIGH);

            prevState = state;
            state = HandleBoiler(state, &sensors, NULL);

            // --- STATE TRANSITION LOG ---
            if (prevState != state)
            {
                ESP_LOGI(TAG, "STATE CHANGE: %s → %s",
                         GetStateString(prevState),
                         GetStateString(state));
            }

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
                cloudSendDataFeedBack = cloudHandler->SendData(rec.temperature, rec.pressure, sensors.level, rec.levelHigh, rec.heater, rec.pump, rec.state, rec.error);
            }

            if (cloudSendDataFeedBack == false)
            {
                ESP_LOGW(TAG, "Cloud Failed, Data Stored in Flash");
                StoreToFlash(&rec);
            }
            else
            {
                ESP_LOGI(TAG, "Cloud OK Send Stored Data");
                SendStoredData();
            }

            ESP_LOGI(TAG,"Temp: %.2f | Press: %.2f | Level: %d | State:%s | Error:%s", rec.temperature, rec.pressure, sensors.level, GetStateString(rec.state), GetErrorString(rec.error));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
