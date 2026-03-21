
#include "CloudService.h"
#include "DebugMessage.h"

#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"

/* ================= CONFIG ================= */

#define THINGSPEAK_API_KEY "6Y9LPBBT7G259TY8"
#define TALKBACK_API_KEY   "8KPPYSG8AHE2ELAI"
#define TALKBACK_ID        "56596"

static const char *TAG = "CLOUD";
static WifiHandler *wifiHandler = NULL;

/* ================= FUNCTION DECL ================= */

static bool Init(void);
bool SendData(float temprature, float pressure, int levelLow, int levelHigh,
              int heater, int pump, int state, int error);
static bool ReadCommand(bool *start);
static void DeleteCommand(int command_id);

/* ================= HANDLER ================= */

static CloudServiceHandler cloudHandler =
{
    Init,
    SendData,
    ReadCommand
};

CloudServiceHandler* CreateCloudService(WifiHandler *wifiHandlerObject)
{
    if (wifiHandlerObject != NULL)
    {
        wifiHandler = wifiHandlerObject;
    }
    return &cloudHandler;
}

/* ================= INIT ================= */

static bool Init(void)
{
    ESP_LOGI(TAG, "Cloud Service Initialized");
    return true;
}

/* ================= SEND DATA ================= */

bool SendData(float temprature, float pressure, int levelLow, int levelHigh,
              int heater, int pump, int state, int error)
{
    char url[256];

    snprintf(url, sizeof(url),
        "http://api.thingspeak.com/update?api_key=%s&field1=%.2f&field2=%.2f&field3=%d&field4=%d&field5=%d&field6=%d&field7=%d&field8=%d",
        THINGSPEAK_API_KEY,
        temprature, pressure,
        levelLow, levelHigh,
        heater, pump,
        state, error);

    esp_http_client_config_t config =
    {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status = esp_http_client_get_status_code(client);

        if (status == 200)
        {
            ESP_LOGI(TAG, "ThingSpeak update success");
            esp_http_client_cleanup(client);
            return true;
        }
        else
        {
            ESP_LOGW(TAG, "HTTP Status: %d", status);
        }
    }
    else
    {
        ESP_LOGE(TAG, "HTTP Request Failed");
    }

    esp_http_client_cleanup(client);
    return false;
}

/* ================= READ COMMAND ================= */

static bool ReadCommand(bool *start)
{
    char url[128];

    snprintf(url, sizeof(url),
             "http://api.thingspeak.com/talkbacks/%s/commands.json?api_key=%s",
             TALKBACK_ID, TALKBACK_API_KEY);

    esp_http_client_config_t config =
    {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    ESP_LOGI(TAG, "Request URL: %s", url);

    esp_err_t err = esp_http_client_open(client, 0);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }

    esp_http_client_fetch_headers(client);

    int status_code = esp_http_client_get_status_code(client);

    if (status_code != 200)
    {
        ESP_LOGE(TAG, "HTTP Status: %d", status_code);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }

    /* ==== READ RESPONSE ==== */

    char buffer[1024] = {0};
    int total_read = 0;
    int read_len;

    while ((read_len = esp_http_client_read(client,
                                            buffer + total_read,
                                            sizeof(buffer) - total_read - 1)) > 0)
    {
        total_read += read_len;

        if (total_read >= sizeof(buffer) - 1)
        {
            ESP_LOGW(TAG, "Response truncated");
            break;
        }
    }

    buffer[total_read] = 0;

    ESP_LOGI(TAG, "Response Length: %d", total_read);

    if (total_read == 0 || strlen(buffer) < 5)
    {
        ESP_LOGI(TAG, "No command available");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return true;
    }

    /* ==== PARSE JSON ==== */

    cJSON *json = cJSON_Parse(buffer);

    if (json == NULL)
    {
        ESP_LOGE(TAG, "JSON parse failed");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }

    if (!cJSON_IsArray(json))
    {
        ESP_LOGE(TAG, "Invalid JSON format");
        cJSON_Delete(json);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }

    int count = cJSON_GetArraySize(json);

    if (count == 0)
    {
        ESP_LOGI(TAG, "No commands in queue");
        cJSON_Delete(json);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return true;
    }

    ESP_LOGI(TAG, "Processing %d commands", count);

    /* ==== PROCESS ALL COMMANDS ==== */

    for (int i = 0; i < count; i++)
    {
        cJSON *item = cJSON_GetArrayItem(json, i);

        if (!cJSON_IsObject(item))
            continue;

        cJSON *id_item = cJSON_GetObjectItem(item, "id");
        cJSON *cmd = cJSON_GetObjectItem(item, "command_string");

        if (!id_item || !cmd || !cmd->valuestring)
            continue;

        int command_id = id_item->valueint;
        const char *command = cmd->valuestring;

        ESP_LOGI(TAG, "CMD ID: %d | CMD: %s", command_id, command);

        /* ==== COMMAND HANDLING ==== */

        if (strcmp(command, "START") == 0)
        {
            ESP_LOGI(TAG, "START received");
            if (start) *start = true;
        }
        else if (strcmp(command, "STOP") == 0)
        {
            ESP_LOGI(TAG, "STOP received");
            if (start) *start = false;
        }
        else if (strcmp(command, "OFF_WIFI") == 0)
        {
             DeleteCommand(command_id);
            if (wifiHandler->Disconnect() != NULL)
            {
                ESP_LOGI(TAG, "WiFi Disconnected Successfully");
            }
        }
        else if (strcmp(command, "ErrorReset") == 0)
        {
            ESP_LOGI("SYSTEM", "System restart initiated...");

            DeleteCommand(command_id);
            // Optional: small delay to flush logs / HTTP
            vTaskDelay(pdMS_TO_TICKS(500));

            esp_restart();
            
        }
        else
        {
            ESP_LOGW(TAG, "Unknown command");
        }

        /* ==== DELETE AFTER PROCESS ==== */
        DeleteCommand(command_id);
    }

    cJSON_Delete(json);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return true;
}

/* ================= DELETE COMMAND ================= */

static void DeleteCommand(int command_id)
{
    char url[128];

    snprintf(url, sizeof(url),
             "http://api.thingspeak.com/talkbacks/%s/commands/%d.json?api_key=%s",
             TALKBACK_ID, command_id, TALKBACK_API_KEY);

    esp_http_client_config_t config =
    {
        .url = url,
        .method = HTTP_METHOD_DELETE,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    ESP_LOGI(TAG, "Deleting CMD ID: %d", command_id);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Command deleted");
    }
    else
    {
        ESP_LOGE(TAG, "Delete failed");
    }

    esp_http_client_cleanup(client);
}