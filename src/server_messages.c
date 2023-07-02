#include "server_messages.h"
#include <stdlib.h>
#include <stdio.h>

MESSAGE_HEADER* parse_header(char* partial_message) {
    int message_type, message_length;

    int res = sscanf(partial_message, "Header\nmessage-type<::::>%d\nmessage-length<::::>%d", &message_type, &message_length);
    if(res < 2) {
        return NULL;
    }

    MESSAGE_HEADER* header = malloc(sizeof(MESSAGE_HEADER));
    header->message_type = message_type;
    header->message_length = message_length;

    return header;
}

MESSAGE* parse_message(int socket_fd, char* message) {
    MESSAGE_HEADER* header = parse_header(message);
    if(header == NULL) {
        return NULL;
    }

    srandom(time(NULL));
    MESSAGE* server_message = malloc(sizeof(MESSAGE));
    message->id = socket_fd * 1000 + (random() % 1000);
    message->header = header;
    message->content = (char*)message + (strlen(message) - header->message_length);

    return server_message;
}