#ifndef READ_TASKS_H
#define READ_TASKS_H

#include "global.h"
#include "server_messages.h"

typedef struct {
    MESSAGE_HEADER* header;
    char* initial_read_message;
    int client_fd;
} READ_BIG_MESSAGE_TASK_ARGS;

OPERATION_STATUS create_read_big_message_task(READ_BIG_MESSAGE_TASK_ARGS* args);

#endif // READ_TASKS_H