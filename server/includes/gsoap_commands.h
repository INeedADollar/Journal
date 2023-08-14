//gsoap_commands.h

#include "commands.h"


command_result* ns__generate_id(int client_fd);

command_result* ns__create_journal(user_id id, char* journal_name);

command_result* retrieve_journal(user_id id, char* journal_name);

command_result* import_journal(user_id id, char* journal_name, char* journal_data, size_t journal_data_size);

command_result* modify_journal(user_id id, char* journal_name, char* new_content, size_t new_content_size);

command_result* delete_journal(user_id id, char* journal_name);

operation_status disconnect_client(user_id id, int client_fd);

