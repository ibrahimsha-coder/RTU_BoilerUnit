#include "Pump.h"
#include "DebugMessage.h"

static bool PumpOn(void);
static bool PumpOff(void);

static PumpHandler pumpHandler =
{
    PumpOn,
    PumpOff
};

PumpHandler *CreatePump(void)
{
    return &pumpHandler;
}

static bool PumpOn(void)
{
    return true;
}

static bool PumpOff(void)
{
    return true;
}
