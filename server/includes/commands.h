#ifndef COMMANDS_H
#define COMMANDS_H

#include "message.h"
#include "utils.h"

typedef struct {
    operation_status status;
    char status_message[1000];
    char* additional_data;
    size_t additional_data_size;
} command_result;

command_result generate_id(message* message);

command_result create_journal(message* message);

command_result retrieve_journal(message* message);

command_result import_journal(message* message);

command_result modify_journal(message* message);

command_result delete_journal(message* message);

command_result disconnect_client(message* message);

#endif // COMMANDS_H