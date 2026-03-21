#ifndef PUMP_HANDLER_H
#define PUMP_HANDLER_H

#include <stdbool.h>

typedef struct PumpHandler
{
    bool (*PumpOn)(void);
    bool (*PumpOff)(void);

} PumpHandler;

#endif//PUMP_HANDLER_H