#ifndef SERVER_QUEUE_H
#define SERVER_QUEUE_H

#include "server_messages.h"
#include "global.h"

OPERATION_STATUS enqueue_message(MESSAGE* message);

MESSAGE* dequeue_message();

#endif // SERVER_QUEUE_H