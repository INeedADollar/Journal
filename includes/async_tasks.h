#ifndef ASYNC_TASKS_H
#define ASYNC_TASKS_H

#include "server_queue.h"
#include "server_messages.h"

#include <C-Thread-Pool/thpool.h>

OPERATION_STATUS create_async_task(MESSAGE* message);

int tasks_running_count();

#endif // ASYNC_TASKS_H