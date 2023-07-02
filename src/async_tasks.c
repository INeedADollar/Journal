#include "async_tasks.h"

typedef struct {
    MESSAGE_HEADER* header;
    char* initial_read_message;
} TASK_ARGS;

static threadpool async_tasks_pool = thpool_init(100);

void task_function(MESSAGE* message) {
    switch(message->header->message_type) {
    case RETRIEVE_JOURNAL:
        retrieve_journal(message);
        break;
    case IMPORT_JOURNAL:
        import_hournal(message);
        break;
    case MODIFY_JOURNAL:
        modify_journal(message);
        break;
    }
}

OPERATION_STATUS create_async_task(MESSAGE* message) {
    OPERATION_STATUS status = (OPERATION_STATUS)thpool_add_work(async_tasks_pool, (void*)task_function, (void*)message);
    return status;
}

int tasks_running_count() {
    return thpool_num_threads_working(async_tasks_pool);
}