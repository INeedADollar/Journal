#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "server_messages.h"
#include "global.h"

OPERATION_STATUS generate_id(MESSAGE_HEADER* header, int socket_fd);

OPERATION_STATUS create_journal(MESSAGE_HEADER* message_header, char* message_part, int socket_fd);

OPERATION_STATUS retrieve_journal(MESSAGE* message);

OPERATION_STATUS import_hournal(MESSAGE* message);

OPERATION_STATUS modify_journal(MESSAGE* message);

OPERATION_STATUS delete_journal(MESSAGE_HEADER* message_header, char* message_part, int socket_fd);

OPERATION_STATUS disconnect_client(MESSAGE_HEADER* header, int sochet_fd);

#endif // OPERATIONS_H