#include "async_tasks.h"
#include "commands.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>


static threadpool async_tasks_pool;

void init_thread_pool() {
    async_tasks_pool = thpool_init(100);
}


void retrieve_journal_task(retrieve_journal_args* args) {
    command_result* result = retrieve_journal(args->usr_id, args->journal_name);
    send_command_result_message(args->message_id / 1000, args->usr_id, result);

    free(args->journal_name);
    free(args);
}


operation_status create_retrieve_journal_task(user_id id, message_id msg_id, char* journal_name) {
    retrieve_journal_args* args = (retrieve_journal_args*)malloc(sizeof(retrieve_journal_args));
    args->usr_id = id;
    args->message_id = msg_id;
    args->journal_name = (char*)malloc(strlen(journal_name) + 1);
    strcpy(args->journal_name, journal_name);

    return (operation_status)thpool_add_work(async_tasks_pool, (void*)retrieve_journal_task, (void*)args);
}


void import_journal_task(import_journal_args* args) {
    command_result* result = import_journal(args->usr_id, args->journal_name, args->journal_content, args->journal_content_size);
    send_command_result_message(args->message_id / 1000, args->usr_id, result);

    free(args->journal_name);
    free(args->journal_content);
    free(args);
}

operation_status create_import_journal_task(user_id id, message_id msg_id, char* journal_name, char* journal_data, size_t journal_data_size) {
    import_journal_args* args = (import_journal_args*)malloc(sizeof(import_journal_args));
    args->usr_id = id;
    args->message_id = msg_id;
    args->journal_content_size = journal_data_size;

    args->journal_name = (char*)malloc(strlen(journal_name) + 1);
    strcpy(args->journal_name, journal_name);

    args->journal_content = (char*)malloc(journal_data_size);
    strcpy(args->journal_content, journal_data);

    return (operation_status)thpool_add_work(async_tasks_pool, (void*)import_journal_task, (void*)args);
}


void modify_journal_task(modify_journal_args* args) {
    command_result* result = modify_journal(args->usr_id, args->journal_name, args->new_content, args->new_content_size);
    send_command_result_message(args->message_id / 1000, args->usr_id, result);

    free(args->journal_name);
    free(args->new_content);
    free(args);
}

operation_status create_modify_journal_task(user_id id, message_id msg_id, char* journal_name, char* new_content, size_t new_content_size) {
    modify_journal_args* args = (modify_journal_args*)malloc(sizeof(modify_journal_args));
    args->usr_id = id;
    args->message_id = msg_id;
    args->new_content_size = new_content_size;

    args->journal_name = (char*)malloc(strlen(journal_name) + 1);
    strcpy(args->journal_name, journal_name);

    args->new_content = (char*)malloc(new_content_size);
    strcpy(args->new_content, new_content);

    return (operation_status)thpool_add_work(async_tasks_pool, (void*)modify_journal_task, (void*)args);
}


int tasks_running_count() {
    return thpool_num_threads_working(async_tasks_pool);
}


void destroy_thread_pool() {
    thpool_destroy(async_tasks_pool);
}
