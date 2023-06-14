#include "server_queue.h"

int enqueue_message(ServerMessage* message) {
    queue->messages[queue->length] = message;
    queue->length++;
}

ServerMessage* dequeue_message() {
    ServerMessage* message = queue->messages[0];
    memmove((void*)queue->messages, (void*)queue->messages + 1, queue->length - 2);
    queue->length--;
    
    return message;
}