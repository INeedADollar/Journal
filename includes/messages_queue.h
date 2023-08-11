#ifndef MESSAGES_QUEUE_H
#define MESSAGES_QUEUE_H

#include "messages.h"
#include "utils.h"

operation_status enqueue_message(message* message);

message* dequeue_message();

#endif // MESSAGES_QUEUE_H