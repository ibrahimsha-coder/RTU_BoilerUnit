#include "WifiDefinitions.h"
#include "AWSCloudService.h"
#include "DebugMessage.h"

#include <string.h>

#include "mqtt_client.h"
#include "cJSON.h"
#include "esp_log.h"
#include "esp_event.h"

/* ================= CONFIG ================= */

#define AWS_ENDPOINT "mqtts://ay2jqszs03lk1-ats.iot.ap-south-1.amazonaws.com"
#define PUB_TOPIC    "rtu/data"
#define CTRL_TOPIC   "rtu/control"

/* ================= CERT ================= */

extern const uint8_t root_cert_auth_crt_start[] asm("_binary_root_cert_auth_crt_start");
extern const uint8_t client_crt_start[] asm("_binary_client_crt_start");
extern const uint8_t client_key_start[] asm("_binary_client_key_start");

/* ========================================= */

static const char *TAG = "AWS_CLOUD";

static WifiHandler *wifiHandler = NULL;
static esp_mqtt_client_handle_t client = NULL;
static bool start_state = false;
static bool mqtt_connected = false;

/* ================= FUNCTION PROTOTYPES ================= */

static bool Init(void);
static bool SendData(float temprature, float pressure, int levelLow, int levelHigh, int heater, int pump, int state, int error);
static bool ReadCommand(bool *start);
static void mqtt_event_handler(void *args, esp_event_base_t base, int32_t event_id, void *event_data);

/* ================= OBJECT ================= */

static AWSCloudServiceHandler awsHandler =
{
    Init,
    SendData,
    ReadCommand
};

/* ================= CREATE ================= */

AWSCloudServiceHandler* CreateAWSCloudService(WifiHandler *wifiHandlerObject)
{
    if (wifiHandlerObject)
    {
        wifiHandler = wifiHandlerObject;
    }

    return &awsHandler;
}

/* ================= INIT ================= */

static bool Init(void)
{
    ESP_LOGI(TAG, "🚀 AWS INIT CALLED");

    if (wifiHandler && wifiHandler->GetStatus() != WIFI_STATUS_CONNECTED)
    {
        ESP_LOGW(TAG, "⚠️ WiFi not connected yet");
    }

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = AWS_ENDPOINT,
        .broker.verification.certificate = (const char *)root_cert_auth_crt_start,
        .credentials.authentication.certificate = (const char *)client_crt_start,
        .credentials.authentication.key = (const char *)client_key_start,
    };

    client = esp_mqtt_client_init(&cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    return true;
}

/* ================= SEND DATA ================= */

static bool SendData(float temprature, float pressure, int levelLow, int levelHigh,
                     int heater, int pump, int state, int error)
{
    if (client == NULL || !mqtt_connected)
    {
        ESP_LOGW(TAG, "⚠️ MQTT not ready");
        return false;
    }

    char payload[256];

    snprintf(payload, sizeof(payload),
        "{\"temperature\":%.2f,\"pressure\":%.2f,\"levelLow\":%d,"
        "\"levelHigh\":%d,\"heater\":%d,\"pump\":%d,\"state\":%d,\"error\":%d}",
        temprature, pressure,
        levelLow, levelHigh,
        heater, pump,
        state, error);

    ESP_LOGI(TAG, "📤 TX Topic: %s", PUB_TOPIC);
    ESP_LOGI(TAG, "📤 TX Data : %s", payload);

    int msg_id = esp_mqtt_client_publish(client, PUB_TOPIC, payload, 0, 1, 0);

    ESP_LOGI(TAG, "📤 Publish msg_id = %d", msg_id);

    return true;
}

/* ================= READ COMMAND ================= */

static bool ReadCommand(bool *start)
{
    if (start)
        *start = start_state;

    return true;
}

/* ================= MQTT EVENT ================= */

static void mqtt_event_handler(void *args, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id)
    {
        case MQTT_EVENT_CONNECTED:
            mqtt_connected = true;
            ESP_LOGI(TAG, "✅ MQTT CONNECTED to AWS");
            esp_mqtt_client_subscribe(client, CTRL_TOPIC, 1);
            ESP_LOGI(TAG, "📡 SUBSCRIBED to topic: %s", CTRL_TOPIC);
            break;

        case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            ESP_LOGW(TAG, "⚠️ MQTT DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "📥 SUBSCRIBE SUCCESS, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "📤 PUBLISH SUCCESS, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
        {
            char topic[event->topic_len + 1];
            char data[event->data_len + 1];

            memcpy(topic, event->topic, event->topic_len);
            topic[event->topic_len] = 0;

            memcpy(data, event->data, event->data_len);
            data[event->data_len] = 0;

            ESP_LOGI(TAG, "📥 RX Topic: %s", topic);
            ESP_LOGI(TAG, "📥 RX Data : %s", data);

            cJSON *json = cJSON_Parse(data);
            if (!json)
            {
                ESP_LOGE(TAG, "❌ JSON parse failed");
                return;
            }

            cJSON *start = cJSON_GetObjectItem(json, "start");
            if (cJSON_IsNumber(start))
            {
                start_state = start->valueint;
                ESP_LOGI(TAG, "⚙️ CMD start = %d", start_state);
            }

            cJSON_Delete(json);
            break;
        }

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "❌ MQTT ERROR");
            break;

        default:
            break;
    }
}