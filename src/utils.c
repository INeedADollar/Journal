#include "utils.h"
#include "logger.h"

#include <sys/socket.h>
#include <stdio.h>


void send_message(message* message) {
    int response_len = message->header->message_length + 100;
    char response[response_len];
    char* content = NULL;

    message_content_node* node = message->content->head;
    while(node) {
        if(content) {
            content = realloc(content, node->node_data->size);
        }
        else {
            content = malloc(node->node_data->size);
        }

        memcpy((void*)content, (void*)node->node_data->value, node->node_data->size);
        node = node->next;
    }

    sprintf(response, "Header\nmessage-type<::::>%d\nmessage-length<::::>%d\nuser-id<::::>%d\nContent\n%s",
        message->header->message_type, message->header->message_length, message->header->user_id, message->content);
    
    ssize_t res = send(message->id / 1000, response, strlen((char*)response), 0);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(res, "Failed to send actual response", message->header->type
        "Failed to send actual response to client with id %lu", message->header->client_id);
        
    LOG_INFO("Message %s sent succesfully to client with id %d", response, message->header->client_id);
}


OPERATION_STATUS send_response(MESSAGE* response) {
    OPERATION_STATUS status = send_message(response);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Error processing your request.", response->header->message_type, 
        "Response for message with type %d could not be sent. Error: %s", response->header->message_type)
    delete_message(response);
    return status;
}


void send_status_message(USER_ID id, char* message_str, MESSAGE_TYPES type, OPERATION_STATUS status) {
    char* keys[] = {
        "status",
        "message"
    };

    char status_str[1];
    itoa((int)status, status_str, 10);

    char* values[] = {
        status_str,
        message_str
    };

    MESSAGE* response_message = (MESSAGE*)calloc(sizeof(MESSAGE));
    response_message->header = (MESSAGE_HEADER*)malloc(sizeof(MESSAGE_HEADER));
    response_message->header->message_type = type;
    response_message->header->user_id = id;
    
    MESSAGE_CONTENT* content = create_message_content(keys, 2, values, 2);
    response_message->content = content;

    send_response(response_message);
}


MESSAGE_CONTENT_NODE_VALUE* extract_value_from_content(MESSAGE_CONTENT* content, char* key) {
    if(!content) {
        return NULL;
    }

    MESSAGE_CONTENT_NODE* node = content->head;
    while(node) {
        if(strcmp(node->key, key) == 0) {
            return node;
        }
    }

    return NULL;
}