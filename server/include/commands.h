#ifndef COMMANDS_H
#define COMMANDS_H

#include "server_defs.h"


command_result* check_message_and_run_command(message_t* message);

command_result* generate_id(int client_fd);

command_result* create_journal(user_id id, char* journal_name);

command_result* retrieve_journal(user_id id, char* journal_name);

command_result* retrieve_journals(user_id id);

command_result* import_journal(user_id id, char* journal_name, char* journal_data, size_t journal_data_size);

command_result* modify_journal(user_id id, char* journal_name, char* new_content, size_t new_content_size);

command_result* delete_journal(user_id id, char* journal_name);

operation_status disconnect_client(user_id id, int client_fd);


#endif // COMMANDS_H