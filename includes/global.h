#ifndef GLOBAL_H
#define GLOBAL_H

#include "server_messages.h"

typedef enum {
    OPERATION_SUCCESS = 0,
    OPERATION_FAIL = -1
} OPERATION_STATUS;

static int STOP_SERVER = 0;

void send_message(MESSAGE* message);

#endif // GLOBAL_H