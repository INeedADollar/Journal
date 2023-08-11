#ifndef ASYNC_TASKS_H
#define ASYNC_TASKS_H

#include "message.h"
#include <C-Thread-Pool/thpool.h>

operation_status create_async_task(message* message);

int tasks_running_count();

#endif // ASYNC_TASKS_H