

#ifndef WIFI_DEFINITIONS_H
#define WIFI_DEFINITIONS_H

typedef enum WifiErrorCode
{
    WifiOk,
    WifiNotInit,
    WifiFail
} WifiErrorCode;

typedef enum WifiStatus
{
    WIFI_STATUS_DISCONNECTED,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED
} WifiStatus;

#endif // WIFI_DEFINITIONS_H
