#include "server_queue.h"

#include <stdlib.h>

typedef struct {
    MESSAGE* messages[1000];
    int length;
} SERVER_QUEUE;

static SERVER_QUEUE* queue = calloc(sizeof(MESSAGE));

int enqueue_message(MESSAGE* message) {
    if(queue->length < 1000) {
        queue->messages[queue->length] = message;
        queue->length++;

        return 0;
    }

    return -1;
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