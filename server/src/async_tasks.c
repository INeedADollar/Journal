#include "async_tasks.h"
#include "commands.h"

static threadpool async_tasks_pool = thpool_init(100);

void task_function(message* message) {
    switch(message->header->type) {
    case RETRIEVE_JOURNAL:
        retrieve_journal(message);
        break;
    case IMPORT_JOURNAL:
        import_hournal(message);
        break;
    case MODIFY_JOURNAL:
        modify_journal(message);
        break;
    default:
        break;
    }
}

operation_status create_async_task(message* message) {
    operation_status status = (operation_status)thpool_add_work(async_tasks_pool, (void*)task_function, (void*)message);
    return status;
}

int tasks_running_count() {
    return thpool_num_threads_working(async_tasks_pool);
}