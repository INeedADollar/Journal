#ifndef ASYNC_TASKS_H
#define ASYNC_TASKS_H

#include "server_queue.h"
#include <C-Thread-Pool/thpool.h>

int create_threadpool_task_from_message(char* initial_read_message);

#endif // ASYNC_TASKS_H