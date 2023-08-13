#ifndef ASYNC_TASKS_H
#define ASYNC_TASKS_H

#include "message.h"
#include <C-Thread-Pool/thpool.h>

typedef struct {
    user_id id;
    char* journal_name;
} retrieve_journal_args;

typedef struct {
    user_id id;
    size_t journal_content_size;
    char* journal_name;
    char* journal_content; 
} import_journal_args;

typedef struct {
    user_id id;
    size_t new_content_size;
    char* journal_name;
    char* new_content;
} modify_journal_args;

void init_thread_pool();

operation_status create_retrieve_journal_task(user_id id, char* journal_name);

operation_status create_import_journal_task(user_id id, char* journal_name, char* journal_data, size_t journal_data_size);

operation_status create_modify_journal_task(user_id id, char* journal_name, char* new_content, size_t new_content_size);

int tasks_running_count();

#endif // ASYNC_TASKS_H