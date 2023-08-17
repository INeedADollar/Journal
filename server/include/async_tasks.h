#ifndef ASYNC_TASKS_H
#define ASYNC_TASKS_H

#include "server_defs.h"
#include "thpool.h"


void init_thread_pool();

operation_status create_retrieve_journal_task(user_id id, char* journal_name);

operation_status create_import_journal_task(user_id id, char* journal_name, char* journal_data, size_t journal_data_size);

operation_status create_modify_journal_task(user_id id, char* journal_name, char* new_content, size_t new_content_size);

int tasks_running_count();

void destroy_thread_pool();


#endif // ASYNC_TASKS_H