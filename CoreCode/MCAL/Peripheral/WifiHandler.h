#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include "WifiDefinitions.h"

typedef struct WifiHandler
{
    WifiErrorCode (*Init)();
    WifiErrorCode (*Connect)(const char *ssid, const char *password);
    WifiStatus (*GetStatus)(void);
    WifiErrorCode (*Disconnect)(void);
} WifiHandler;

#endif // WIFI_HANDLER_H
