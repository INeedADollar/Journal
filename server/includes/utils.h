#ifndef UTILS_H
#define UTILS_H

#include "commands.h"

typedef enum {
    OPERATION_SUCCESS = 0,
    OPERATION_FAIL = -1
} operation_status;

static int STOP_SERVER = 0;

void send_command_result_message(command_result* result);

#endif // UTILS_H