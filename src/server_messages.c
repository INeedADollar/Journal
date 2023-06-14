#include "server_messages.h"
#include <stdlib.h>
#include <stdio.h>

Header* parse_header(char* partial_message) {
    int message_length;
    char message_type[10];

    int res = sscanf(partial_message, "Header\nmessage-length<::::>%d\nmessage-type<::::>%10s\n", &message_length, message_type);
    if(res < 2) {
        return NULL;
    }

    Header* header = malloc(sizeof(Header));
    header->message_length = message_length;
    header->message_type = get_message_type(message_type);

    return header;
}

ServerMessage* parse_message(int socket_fd, char* message) {
    Header* header = parse_header(message);
    if(header == NULL) {
        return NULL;
    }

    srandom(time(NULL));
    ServerMessage* server_message = malloc(sizeof(ServerMessage));
    message->id = socket_fd * 1000 + (random() % 1000);
    message->header = header;
    message->content = (char*)message + (strlen(message) - header->message_length);

    return server_message;
}