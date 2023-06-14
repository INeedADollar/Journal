#ifndef SERVER_QUEUE_H
#define SERVER_QUEUE_H

#include <stdlib.h>

typedef struct Queue {
    ServerMessage* messages[1000];
    int length;
} ServerQueue;

int enqueue_message(ServerMessage* message);

ServerMessage* dequeue_message();

#endif // SERVER_QUEUE_H