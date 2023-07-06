#ifndef COMMANDS_H
#define COMMANDS_H

#include "server_messages.h"
#include "global.h"

OPERATION_STATUS generate_id(MESSAGE* message);

OPERATION_STATUS create_journal(MESSAGE* message);

OPERATION_STATUS retrieve_journal(MESSAGE* message);

OPERATION_STATUS import_journal(MESSAGE* message);

OPERATION_STATUS modify_journal(MESSAGE* message);

OPERATION_STATUS delete_journal(MESSAGE* message);

OPERATION_STATUS disconnect_client(MESSAGE* message);

#endif // COMMANDS_HE