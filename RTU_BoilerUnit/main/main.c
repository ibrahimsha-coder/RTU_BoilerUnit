#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "TempratureSensorHandler.h"
#include "WIFI.h"
#include "TempratureSensor.h"
#include "PressureSensor.h"
#include "LevelSensor.h"
#include "Pump.h"
#include "Heater.h"
#include "BoilerUnit.h"
#include "CloudService.h"
#include "RTUProcess.h"
#include "DebugMessage.h"

static WifiHandler *espWifi = NULL;
static TempratureSensorHandler *tempratureSensor = NULL;
static PressureSensorHandler *pressureSensor = NULL;
static LevelSensorHandler *levelSensor = NULL;
static PumpHandler *pump = NULL;
static HeaterHandler *heater = NULL;
static BoilerUnitHandler *boilerUnit = NULL;
static CloudServiceHandler *cloud = NULL;

static bool CreateMCAL(void);
static bool CreatePlatform(void);
static bool CreateServices(void);
static bool CreateApplication(void);

static bool CreatePeripherals(void);
static bool CreatePlatform(void);
static bool CreateSensorsAndDevices(void);
static bool CreateInternalServices(void);
static bool CreateExternalServices(void);
static bool CreateSubSystems(void);
static bool CreateProcess(void);

void app_main()
{    
    if (CreateMCAL() == true)
    {	
        if (CreatePlatform() == true)
        {
            if (CreateServices() == true)
            {
                if (CreateApplication() == true)
                {
                    DEBUG_LOG_MSG("%s", "CreateApplication Success");
                }            
            }
        }
    }
}

static bool CreateMCAL(void)
{
    bool retVal = false;

    if (CreatePeripherals() != false)
    {
        retVal = true;
    }

    return retVal;
}

static bool CreatePlatform()
{
    bool retVal = false;

    if (CreateSensorsAndDevices() != false)
    {
        retVal = true;        
    }
    return retVal;
}

static bool CreateServices(void)
{
    bool retVal = false;

    if (CreateInternalServices() != false)
    {
        if (CreateExternalServices() != false)
        {
            retVal = true;
        }    
    }


    return retVal;
}

static bool CreateApplication(void)
{
    bool retVal = false;
    
    if (CreateProcess() != false)
    {
        retVal = true;
    }
    return retVal;
}

static bool CreatePeripherals(void)
{
    bool retVal = false;

    espWifi = CreateWifi();

    if (espWifi != NULL)
    {
        if (espWifi->Init() != WifiOk)
        {
            if (espWifi->Connect("SWEET_HOME", "9993763619") != WifiOk)
            {
                retVal = true;
            }
        }
    }

    return true;
}

static bool CreateSensorsAndDevices(void)
{
    bool retVal = false;
    
    tempratureSensor = CreateTempratureSensor();
    
    if (tempratureSensor != NULL)
    {
		pressureSensor = CreatePressureSensor();

		if (pressureSensor != NULL)
		{
            pump = CreatePump();

            if (pump != NULL)
            {
                heater = CreateHeater();

                if (heater != NULL)
                {
                    levelSensor = CreateLevelSensor();

                    if (levelSensor != NULL)
                    {
                        retVal = true;
                    }
                }
            }
		}
	}
    
    return retVal;
    
}

static bool CreateSubSystems(void)
{
	bool retVal = false;

    boilerUnit = CreateBoilerUnit(tempratureSensor, pressureSensor, levelSensor, pump, heater);
    
    if (boilerUnit != NULL)
    {
		retVal = true;
	}
	
	return retVal;
}

static bool CreateInternalServices(void)
{
    bool retVal = false;

    if (CreateSubSystems() == true)
    {
        retVal = true;
    }

    return retVal;
}


static bool CreateExternalServices(void)
{
    bool retVal = false;

    cloud = CreateCloudService();

    if (cloud != NULL)
    {
        retVal = true;
    }

    return retVal;
}

bool CreateProcess(void)
{
    bool retVal = false;

    if (CreateRTUProcess(espWifi, cloud, boilerUnit) != NULL)
    {
        retVal = true;
    }

    return retVal;
}