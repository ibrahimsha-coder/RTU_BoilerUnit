#ifndef AWS_CLOUD_SERVICE_H
#define AWS_CLOUD_SERVICE_H

#include "WifiHandler.h"
#include "AWSCloudServiceHandler.h"

AWSCloudServiceHandler* CreateAWSCloudService(WifiHandler *wifiHandlerObject);

#endif