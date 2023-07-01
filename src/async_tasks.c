#include "async_tasks.h"

static threadpool pool = thpool_init(100);

void task_function(MESSAGE* message) {
    
}

int create_threadpool_task_from_message(MESSAGE* message) {

}