#ifndef RTU_PROCESS_HANDLER_H
#define RTU_PROCESS_HANDLER_H

#include <stdbool.h>

typedef struct RTCProcessHandler
{
    bool (*Init)(void);
} RTCProcessHandler;

#endif //RTU_PROCESS_HANDLER_H