#include "read_tasks.h"
#include "server_queue.h"

#include <C-Thread-Pool/thpool.h>
#include <arpa/inet.h>	
#include <string.h>
#include <stdlib.h>

static threadpool message_tasks = thpool_init(100);

void read_big_message_task(read_big_message_task_args* args) {
    int total_received = 0;
    char part[1024];

    char read_part[strlen(args->initial_read_message)];
    strcpy(read_part, args->initial_read_message);

    char* content_read = strstr(read_part, "Content\n");
    int header_len = 0;

    if(content_read) {
        total_received = strlen(content_read) - 8;
        header_len = strlen(content_read) + 8;
    }

    char* read_message = (char*)malloc(header_len + args->header->message_length);
    strcpy(read_message, args->initial_read_message);

    while((total_received += recv(args->client_fd, &part, 1024, 0)) < args->header->message_length + header_len) {
        strcat(read_message, part);
    }

    read_message[total_received] = '\0';
    message* message = parse_message(read_message);
    if(message) {
        enqueue_message(message);
    }

    free(args->header);
    free(args);
}

operation_status create_read_big_message_task(read_big_message_task_args* args) {
    return (operation_status)thpool_add_work(message_tasks, (void*)read_big_message_task, (void*)args);
}