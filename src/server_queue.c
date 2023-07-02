#include "server_queue.h"

#include <stdlib.h>

typedef struct {
    MESSAGE* messages[1000];
    int length;
} SERVER_QUEUE;

static SERVER_QUEUE* queue = calloc(sizeof(MESSAGE));

OPERATION_STATUS enqueue_message(MESSAGE* message) {
    if(queue->length < 1000) {
        queue->messages[queue->length] = message;
        queue->length++;

        return OPERATION_SUCCESS;
    }

    return OPERATION_FAIL;
}

MESSAGE* dequeue_message() {
    if(queue->length > 0) {
        MESSAGE* message = queue->messages[0];
        memmove((void*)queue->messages, (void*)(queue->messages + 1), queue->length - 2);
        queue->length--;

        return message;
    }

    return (MESSAGE*)NULL;
}