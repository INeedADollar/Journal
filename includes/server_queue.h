#ifndef SERVER_QUEUE_H
#define SERVER_QUEUE_H

#include <stdlib.h>

typedef struct Message {
    int id;
    char* header;
    char* content;
} ServerMessage;

typedef struct Queue {
    ServerMessage* messages[1000];
    int length;
} ServerQueue;

ServerQueue* queue = malloc(sizeof(ServerQueue));

int enqueue_message(ServerMessage* message);

ServerMessage* dequeue_message();

#endif // SERVER_QUEUE_H