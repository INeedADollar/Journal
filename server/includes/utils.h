#ifndef UTILS_H
#define UTILS_H

#include "message.h"

#define CHECK_FOR_FAIL_AND_SEND_MESSAGE(variable, message_error, type, console_error, ...) \
    if(!variable || variable == OPERATION_FAIL) { \
        LOG_ERROR(console_error, __VA_ARGS__, strerror(errno)); \
        send_status_message(message_error, type, OPERATION_FAIL); \
        return OPERATION_FAIL; \
    } \

typedef enum {
    OPERATION_SUCCESS = 0,
    OPERATION_FAIL = -1
} operation_status;

static int STOP_SERVER = 0;

void send_message(message* message);

#endif // UTILS_H