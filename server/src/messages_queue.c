#include "messages_queue.h"

#include <stdlib.h>
#include <string.h>

static messages_queue* queue = NULL;


void initialize_messages_queue() {
    queue = calloc(1, sizeof(messages_queue));
}


operation_status enqueue_message(message_t* message) {
    if(queue->length < 1000) {
        queue->messages[queue->length] = message;
        queue->length++;

        return OPERATION_SUCCESS;
    }

    return OPERATION_FAIL;
}

message_t* dequeue_message() {
    if(queue->length > 0) {
        message_t* message = queue->messages[0];
        memmove((void*)queue->messages, (void*)(queue->messages + 1), queue->length - 2);
        queue->length--;

        return message;
    }

    return (message_t*)NULL;
}


void destroy_messages_queue() {
    free(queue);
}