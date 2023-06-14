#ifndef ASYNC_TASKS_H
#define ASYNC_TASKS_H

#include "./libs/C-Thread-Pool/thpool.h"
#include "server_queue.h"

int create_threadpool_task_from_message(ServerMessage* message);

#endif // ASYNC_TASKS_H