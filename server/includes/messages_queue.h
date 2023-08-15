#ifndef MESSAGES_QUEUE_H
#define MESSAGES_QUEUE_H

#include "server_defs.h"


void initialize_messages_queue();

operation_status enqueue_message(message* message);

message* dequeue_message();

void destroy_messages_queue();


#endif // MESSAGES_QUEUE_H