#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "server_messages.h"

void retrieve_journal(MESSAGE* message);

void import_hournal(MESSAGE* message);

void modify_journal(MESSAGE* message);

#endif // OPERATIONS_H