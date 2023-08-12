#include "utils.h"
#include "logger.h"

#include <sys/socket.h>
#include <stdio.h>


operation_status send_message(message* message) {
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
    return OPERATION_SUCCESS;
}


operation_status send_response(message* response) {
    operation_status status = send_message(response);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Error processing your request.", response->header->message_type, 
        "Response for message with type %d could not be sent. Error: %s", response->header->message_type)
    delete_message(response);
    return status;
}


void send_status_message(user_id id, char* message_str, message_types type, operation_status status) {
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

    message* response_message = (message*)calloc(sizeof(message));
    response_message->header = (message_header*)malloc(sizeof(message_header));
    response_message->header->message_type = type;
    response_message->header->user_id = id;
    
    message_header* content = create_message_content(keys, 2, values, 2);
    response_message->content = content;

    send_response(response_message);
}


message_content_node* extract_value_from_content(message_content* content, char* key) {
    if(!content) {
        return NULL;
    }

    message_content_node* node = content->head;
    while(node) {
        if(strcmp(node->key, key) == 0) {
            return node;
        }
    }

    return NULL;
}