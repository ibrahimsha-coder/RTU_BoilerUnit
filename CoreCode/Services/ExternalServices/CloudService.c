#include "CloudService.h"
#include "DebugMessage.h"

#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"   
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 


#include "cJSON.h"


#define THINGSPEAK_API_KEY "6Y9LPBBT7G259TY8"
#define TALKBACK_API_KEY   "8KPPYSG8AHE2ELAI"
#define TALKBACK_ID        "56596"

static const char *TAG = "CLOUD";

static int last_command_id = -1;

static bool Init(void);
bool SendData(float temprature, float pressure, int levelLow, int levelHigh, int heater, int pump, int state, int error);
static bool ReadCommand(bool *start);

void DeleteCommand(int command_id);

static CloudServiceHandler cloudHandler =
{
    Init,
    SendData,
    ReadCommand
};

CloudServiceHandler* CreateCloudService(void)
{
    return &cloudHandler;
}

static bool Init(void)
{
    ESP_LOGI(TAG, "Cloud Service Initialized");
    return true;
}


/* ================= SEND DATA ================= */

bool SendData(float temprature, float pressure, int levelLow, int levelHigh, int heater, int pump, int state, int error)
{
    char url[256];

    snprintf(url, sizeof(url),
    "http://api.thingspeak.com/update?api_key=%s&field1=%.2f&field2=%.2f&field3=%d&field4=%d&field5=%d&field6=%d&field7=%d&field8=%d",
    THINGSPEAK_API_KEY, temprature, pressure, levelLow, levelHigh, heater, pump, state, error);

    esp_http_client_config_t config =
    {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000
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

// bool ReadCommand(bool *start)
// {
//     char url[128];

// 	snprintf(url, sizeof(url),
// 	"http://api.thingspeak.com/talkbacks/%s/commands.json?api_key=%s",
// 	TALKBACK_ID, TALKBACK_API_KEY);

//     esp_http_client_config_t config = {
//         .url = url,
//         .method = HTTP_METHOD_GET,
//         .timeout_ms = 5000,
//     };

//     esp_http_client_handle_t client = esp_http_client_init(&config);

//     ESP_LOGI(TAG, "Request URL: %s", url);

//     esp_err_t err = esp_http_client_open(client, 0);

//     if (err != ESP_OK)
//     {
//         ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
// 	esp_http_client_close(client);
// 	esp_http_client_cleanup(client);
//         return false;
//     }

// 	int content_length = esp_http_client_fetch_headers(client);
	
// 	int status_code = esp_http_client_get_status_code(client);
// 	ESP_LOGI(TAG, "HTTP Status Code: %d", status_code);
// 	ESP_LOGI(TAG, "Content-Length: %d", content_length);
// 	ESP_LOGI(TAG, "Is Chunked: %d", esp_http_client_is_chunked_response(client));
	
// 	if (status_code != 200)
// 	{
// 	    ESP_LOGE(TAG, "Non-200 status");
// 	    esp_http_client_close(client);
// 	    esp_http_client_cleanup(client);
// 	    return false;
// 	}

// 	char buffer[1024] = {0};
// 	int total_read = 0;
// 	int read_len;
	
// 	// 🔥 Correct read loop for chunked
// 	while ((read_len = esp_http_client_read(client,
// 	                                        buffer + total_read,
// 	                                        sizeof(buffer) - total_read - 1)) > 0)
// 	{
// 	    total_read += read_len;
// 	}
	
// 	buffer[total_read] = 0;
	
	
// 	ESP_LOGI(TAG, "Response Length: %d", total_read);
// 	ESP_LOGI(TAG, "Response: %s", buffer);
	
// 	// ✅ No command case
// 	if (total_read == 0 || strlen(buffer) < 5)
// 	{
// 	    ESP_LOGI(TAG, "No command available");
// 	    esp_http_client_close(client);
// 	    esp_http_client_cleanup(client);
// 	    return true;
// 	}

//     // 🔥 Parse JSON
//     cJSON *json = cJSON_Parse(buffer);
//     if (json == NULL)
//     {
//         ESP_LOGE(TAG, "JSON parse failed");
// 		esp_http_client_close(client);
// 		esp_http_client_cleanup(client);
//         return false;
//     }

// // 🔥 Ensure it's an array
// 	if (!cJSON_IsArray(json))
// 	{
// 	    ESP_LOGE(TAG, "JSON is not array");
// 	    cJSON_Delete(json);
// 		esp_http_client_close(client);
// 		esp_http_client_cleanup(client);
// 	    return false;
// 	}
	
// 	// Get first (latest) command
// 	cJSON *item = cJSON_GetArrayItem(json, 0);
	
// 	if (item == NULL)
// 	{
// 	    ESP_LOGI(TAG, "No command in array");
// 	    cJSON_Delete(json);
// 	    esp_http_client_close(client);
// 	    esp_http_client_cleanup(client);
// 	    return true;
// 	}
	
// 	cJSON *id_item = cJSON_GetObjectItem(item, "id");
// 	cJSON *cmd = cJSON_GetObjectItem(item, "command_string");

//     if (id_item && cmd && cmd->valuestring)
//     {
//         int current_id = id_item->valueint;

//         ESP_LOGI(TAG, "Command ID: %d", current_id);

//         // 🔥 Prevent duplicate execution
//         if (current_id == last_command_id)
//         {
//             ESP_LOGI(TAG, "Command already processed, ignoring");
//         }
//         else
//         {
//             last_command_id = current_id;

//             ESP_LOGI(TAG, "New Command: %s", cmd->valuestring);

//             if (strcmp(cmd->valuestring, "RESET") == 0)
//             {
//                 ESP_LOGI(TAG, "Device RESET triggered");
//                 esp_restart();
//             }
//             else if (strcmp(cmd->valuestring, "START_0") == 0)
//             {
// 				DeleteCommand(current_id);
//                 ESP_LOGI(TAG, "START command received");
//                 if (start)
//                     *start = true;
                    
//             }
//             else if (strcmp(cmd->valuestring, "STOP") == 0)
//             {
//                 ESP_LOGI(TAG, "STOP command received");
//                 if (start)
//                     *start = false;
//             }
//             else
//             {
//                 ESP_LOGW(TAG, "Unknown command");
//             }
//         }
//     }
//     else
//     {
//         ESP_LOGW(TAG, "Invalid JSON or missing fields");
//     }

//     cJSON_Delete(json);
//     esp_http_client_close(client);
//     esp_http_client_cleanup(client);

//     return true;
// }

bool ReadCommand(bool *start)
{
    char url[128];

    snprintf(url, sizeof(url),
             "http://api.thingspeak.com/talkbacks/%s/commands.json?api_key=%s",
             TALKBACK_ID, TALKBACK_API_KEY);

    esp_http_client_config_t config = {
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
    ESP_LOGI(TAG, "HTTP Status Code: %d", status_code);

    if (status_code != 200)
    {
        ESP_LOGE(TAG, "Non-200 status");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }

    // 🔥 Read response
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
            ESP_LOGW(TAG, "Response truncated!");
            break;
        }
    }

    buffer[total_read] = 0;

    ESP_LOGI(TAG, "Response Length: %d", total_read);
    ESP_LOGI(TAG, "Response: %s", buffer);

    if (total_read == 0 || strlen(buffer) < 5)
    {
        ESP_LOGI(TAG, "No command available");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return true;
    }

    // 🔥 Parse JSON
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
        ESP_LOGE(TAG, "JSON is not array");
        cJSON_Delete(json);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }

    int array_size = cJSON_GetArraySize(json);
    ESP_LOGI(TAG, "Command count: %d", array_size);

    if (array_size == 0)
    {
        ESP_LOGI(TAG, "No commands in queue");
        cJSON_Delete(json);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return true;
    }

    // ✅ Get latest command (LAST item)
    cJSON *item = cJSON_GetArrayItem(json, array_size - 1);

    if (item == NULL || !cJSON_IsObject(item))
    {
        ESP_LOGE(TAG, "Invalid command object");
        cJSON_Delete(json);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }

    cJSON *id_item = cJSON_GetObjectItem(item, "id");
    cJSON *cmd = cJSON_GetObjectItem(item, "command_string");

    if (id_item && cmd && cmd->valuestring)
    {
        int current_id = id_item->valueint;

        ESP_LOGI(TAG, "Command ID: %d", current_id);

        // 🔥 Prevent duplicate execution
        if (current_id == last_command_id)
        {
            ESP_LOGI(TAG, "Command already processed, ignoring");
        }
        else
        {
            last_command_id = current_id;

            ESP_LOGI(TAG, "New Command: %s", cmd->valuestring);

            if (strcmp(cmd->valuestring, "RESET") == 0)
            {
                ESP_LOGI(TAG, "Device RESET triggered");

                DeleteCommand(current_id);  // delete before restart
                esp_restart();
            }
            else if (strcmp(cmd->valuestring, "START_0") == 0)
            {
                ESP_LOGI(TAG, "START command received");

                if (start)
                    *start = true;

                DeleteCommand(current_id);
                esp_restart();
            }
            else if (strcmp(cmd->valuestring, "STOP_0") == 0)
            {
                ESP_LOGI(TAG, "STOP command received");

                if (start)
                    *start = false;

                DeleteCommand(current_id);
            }
            else
            {
                ESP_LOGW(TAG, "Unknown command");
            }
        }
    }
    else
    {
        ESP_LOGW(TAG, "Invalid JSON fields");
    }

    cJSON_Delete(json);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return true;
}

void DeleteCommand(int command_id)
{
    char url[128];

    snprintf(url, sizeof(url),
        "http://api.thingspeak.com/talkbacks/%s/commands/%d.json?api_key=%s",
        TALKBACK_ID, command_id, TALKBACK_API_KEY);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_DELETE,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    ESP_LOGI(TAG, "Deleting command ID: %d", command_id);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Command deleted successfully");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to delete command");
    }

    esp_http_client_cleanup(client);
}