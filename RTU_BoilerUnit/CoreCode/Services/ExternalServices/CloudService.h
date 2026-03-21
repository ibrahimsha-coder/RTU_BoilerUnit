#ifndef CLOUD_SERVICE_H
#define CLOUD_SERVICE_H

#include "WifiHandler.h"
#include "CloudServiceHandler.h"

CloudServiceHandler* CreateCloudService(WifiHandler *wifiHandlerObject);

#endif//CLOUD_SERVICE_H