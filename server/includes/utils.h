#ifndef UTILS_H
#define UTILS_H

#include "message.h"

typedef enum {
    OPERATION_SUCCESS = 0,
    OPERATION_FAIL = -1
} operation_status;

static int STOP_SERVER = 0;

void send_message(message* message);

#endif // UTILS_H