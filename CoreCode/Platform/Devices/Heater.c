#include "Heater.h"
#include "DebugMessage.h"

static bool HeaterOn(void);
static bool HeaterOff(void);

static HeaterHandler heaterHandler =
{
    HeaterOn,
    HeaterOff
};

HeaterHandler *CreateHeater(void)
{
    return &heaterHandler;
}

static bool HeaterOn(void)
{
    return true;
}

static bool HeaterOff(void)
{
    return true;
}
