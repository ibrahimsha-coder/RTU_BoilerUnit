

#ifndef DEBUG_MESSAGE_H
#define DEBUG_MESSAGE_H

#include "esp_log.h"

#define ErrorTag ""
#define MessageTag ""

#define DEBUG_LOG_ERR(format, errorData, ...) ESP_LOGE(ErrorTag, "%s %s (%d):" format, __FILE__, __FUNCTION__, __LINE__, errorData, ##__VA_ARGS__)
#define DEBUG_LOG_MSG(format, message, ...) ESP_LOGI(MessageTag, format, message, ##__VA_ARGS__)

#endif // DEBUG_MESSAGE_H
