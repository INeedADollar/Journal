#include "read_tasks.h"
#include "server_queue.h"

#include <C-Thread-Pool/thpool.h>
#include <arpa/inet.h>	
#include <string.h>
#include <stdlib.h>

static threadpool message_tasks = thpool_init(100);

void read_big_message_task(READ_BIG_MESSAGE_TASK_ARGS* args) {
    int total_received = 0;
    char part[1024];

    char read_part[strlen(args->initial_read_message)];
    strcpy(read_part, args->initial_read_message);
    strcat(args->initial_read_message, "\0");

    char* content_read = strstr(read_part, "Content\n");
    int header_len = 0;

    if(content_read) {
        total_received = strlen(content_read) - 8;

        *content_read = '\0';
        header_len = strlen(content_read) + 8;
    }

    char* read_message = (char*)malloc(header_len + args->header->message_length);
    strcpy(read_message, args->initial_read_message);

    while((total_received += recv(args->client_fd, &part, 1024, 0)) < args->header->message_length + header_len) {
        strcat(read_message, part);
    }

    MESSAGE* message = parse_message(read_message);
    if(message) {
        enqueue_message(message);
    }

    free(read_message);
}

OPERATION_STATUS create_read_big_message_task(READ_BIG_MESSAGE_TASK_ARGS* args) {
    thpool_add_work(message_tasks, (void*)read_big_message_task, (void*)args);
}