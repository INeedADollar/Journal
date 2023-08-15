#ifndef UTILS_H
#define UTILS_H

#include "server_defs.h"


operation_status send_command_result_message(user_id id, command_result* result);

message_content_node* extract_value_from_content(message_content* content, char* key);


#endif // UTILS_H