#include "global.h"

#include <sys/socket.h>
#include <stdio.h>

void send_message(MESSAGE* message) {
    int response_len = message->header->message_length + 100;
    char response[response_len];

    sprintf(response, "Header\nmessage-type<::::>%d\nmessage-length<::::>%d\nuser-id<::::>%d\nContent\n%s",
        message->header->message_type, message->header->message_length, message->header->user_id, message->content);
    
    int res = send(message->id / 1000, response, strlen((char*)response), 0);
    if(res == OPERATION_FAIL) {
        perror("Could not send message:")
    }
    else {
        printf("Message %s sent succesfully to client with id %d.\n", response, message->header->user_id);
    }
}