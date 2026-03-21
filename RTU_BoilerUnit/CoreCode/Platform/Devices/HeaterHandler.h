#ifndef HEATER_HANDLER_H
#define HEATER_HANDLER_H

#include <stdbool.h>

typedef struct HeaterHandler
{
    bool (*HeaterOn)(void);
    bool (*HeaterOff)(void);

} HeaterHandler;

#endif//HEATER_HANDLER_H