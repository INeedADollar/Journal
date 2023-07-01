#ifndef SERVER_QUEUE_H
#define SERVER_QUEUE_H

#include <server_messages.h>

void init_queue();

int enqueue_message(ServerMessage* message);

ServerMessage* dequeue_message();

#endif // SERVER_QUEUE_H