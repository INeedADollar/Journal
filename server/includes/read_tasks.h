#ifndef READ_TASKS_H
#define READ_TASKS_H

#include "utils.h"
#include "message.h"

typedef struct {
    message_header* header;
    char* initial_read_message;
    int client_fd;
} read_big_message_task_args;

operation_status create_read_big_message_task(read_big_message_task_args* args);

#endif // READ_TASKS_H