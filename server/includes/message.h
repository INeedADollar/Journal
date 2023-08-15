#ifndef MESSAGES_H
#define MESSAGES_H

#include "server_defs.h"


message_header* parse_header(char* partial_message);

message* parse_message(int socked_fd, char* message);

void delete_message(message* message);

message_content* create_message_content(char* keys[], size_t keys_len, char* values[], size_t values_len);

#endif // MESSAGES_H