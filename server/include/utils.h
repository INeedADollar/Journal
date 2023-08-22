#ifndef UTILS_H
#define UTILS_H

#include "server_defs.h"


void get_command_type_string(command_types type, char* buffer);

operation_status send_command_result_message(int client_fd, command_types type, user_id id, command_result* result);

message_content_node_data* extract_value_from_content(message_content* content, char*suffix);

int string_ends_with(const char * str, const char * suffix);


#endif // UTILS_H