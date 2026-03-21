//
// © 2025 BoatLoadMinds PVT LMT. All Rights Reserved.
//

#include "OS.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi_types.h"
#include "lwip/sockets.h"
#include "esp_mac.h"
#include "lwip/ip4_addr.h"
#include "nvs_flash.h"

#include "WIFI.h"
#include "DebugMessage.h"

static const char *TAG = "WIFI";

/* ================= CONFIG ================= */

#define WIFI_SSID "SWEET_HOME"
#define WIFI_PASS "9993763619"

/* ================= GLOBALS ================= */

static WifiStatus wifi_status = WIFI_STATUS_DISCONNECTED;
static esp_netif_t *sta_netif = NULL;
static bool auto_reconnect = true;
static bool wifi_initialized = false;

/* ================= PROTOTYPES ================= */

static WifiErrorCode Init(void);
static WifiErrorCode Connect(const char *ssid, const char *password);
static WifiStatus GetStatus(void);
static WifiErrorCode Disconnect(void);

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data);

/* ================= HANDLER ================= */

static WifiHandler wifiHandler =
{
    .Init = Init,
    .Connect = Connect,
    .GetStatus = GetStatus,
    .Disconnect = Disconnect
};

WifiHandler *CreateWifi(void)
{
    return &wifiHandler;
}

/* ================= INIT ================= */

static WifiErrorCode Init(void)
{
    if (wifi_initialized)
    {
        ESP_LOGW(TAG, "WiFi already initialized");
        return WifiOk;
    }

    esp_err_t ret;

    /* NVS Init */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    /* Register Events */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL,
        NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &wifi_event_handler,
        NULL,
        NULL));

    /* Default Config */
    wifi_config_t wifi_config = {0};

    strncpy((char *)wifi_config.sta.ssid,
            WIFI_SSID,
            sizeof(wifi_config.sta.ssid) - 1);

    strncpy((char *)wifi_config.sta.password,
            WIFI_PASS,
            sizeof(wifi_config.sta.password) - 1);

    wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';
    wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_ERROR_CHECK(esp_wifi_start());

    wifi_initialized = true;

    ESP_LOGI(TAG, "WiFi Init Completed");

    return WifiOk;
}

/* ================= CONNECT ================= */
/* Optional: use only if changing SSID dynamically */

static WifiErrorCode Connect(const char *ssid, const char *password)
{
    if (!wifi_initialized)
    {
        ESP_LOGE(TAG, "WiFi not initialized");
        return WifiFail;
    }

    wifi_config_t wifiConfig = {0};

    auto_reconnect = true;

    if (strlen(password) == 0)
    {
        wifiConfig.sta.threshold.authmode = WIFI_AUTH_OPEN;
        DEBUG_LOG_MSG("Connecting to SSID: %s (Open)", ssid);
    }
    else
    {
        wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        DEBUG_LOG_MSG("Connecting to SSID: %s", ssid);
    }

    strncpy((char *)wifiConfig.sta.ssid,
            ssid,
            sizeof(wifiConfig.sta.ssid) - 1);

    strncpy((char *)wifiConfig.sta.password,
            password,
            sizeof(wifiConfig.sta.password) - 1);

    wifiConfig.sta.ssid[sizeof(wifiConfig.sta.ssid) - 1] = '\0';
    wifiConfig.sta.password[sizeof(wifiConfig.sta.password) - 1] = '\0';

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig));

    esp_err_t err = esp_wifi_connect();

    if (err == ESP_OK || err == ESP_ERR_WIFI_CONN)
    {
        wifi_status = WIFI_STATUS_CONNECTING;
        return WifiOk;
    }

    ESP_LOGE(TAG, "WiFi connect failed: %d", err);
    return WifiFail;
}

/* ================= STATUS ================= */

static WifiStatus GetStatus(void)
{
    return wifi_status;
}

/* ================= DISCONNECT ================= */

static WifiErrorCode Disconnect(void)
{
    auto_reconnect = false;

    if (esp_wifi_disconnect() == ESP_OK)
    {
        wifi_status = WIFI_STATUS_DISCONNECTED;
        ESP_LOGI(TAG, "WiFi Disconnected");
        return WifiOk;
    }

    ESP_LOGE(TAG, "WiFi Disconnect Failed");
    return WifiFail;
}

/* ================= EVENT HANDLER ================= */

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Connecting...");
        wifi_status = WIFI_STATUS_CONNECTING;
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_status = WIFI_STATUS_DISCONNECTED;

        ESP_LOGW(TAG, "Disconnected");

        if (auto_reconnect)
        {
            ESP_LOGI(TAG, "Reconnecting...");
            wifi_status = WIFI_STATUS_CONNECTING;
            esp_wifi_connect();
        }
    }
    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        wifi_status = WIFI_STATUS_CONNECTED;

        ESP_LOGI(TAG, "Connected");
        ESP_LOGI(TAG, "IP: " IPSTR,
                 IP2STR(&event->ip_info.ip));
    }
}