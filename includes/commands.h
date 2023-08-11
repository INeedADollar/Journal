#ifndef COMMANDS_H
#define COMMANDS_H

#include "message.h"
#include "utils.h"

operation_status generate_id(message* message);

operation_status create_journal(message* message);

operation_status retrieve_journal(message* message);

operation_status import_journal(message* message);

operation_status modify_journal(message* message);

operation_status delete_journal(message* message);

operation_status disconnect_client(message* message);

#endif // COMMANDS_H