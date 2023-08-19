#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#include "message.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

    
command_types get_command_type(char* type_str) {
    if(strcmp(type_str, "GENERATE_ID") == 0) {
        return GENERATE_ID;
    }

    if(strcmp(type_str, "CREATE_JOURNAL") == 0) {
        return CREATE_JOURNAL;
    }

    if(strcmp(type_str, "RETRIEVE_JOURNAL") == 0) {
        return RETRIEVE_JOURNAL;
    }
    
    if(strcmp(type_str, "RETRIEVE_JOURNALS") == 0) {
        return RETRIEVE_JOURNALS;
    }

    if(strcmp(type_str, "IMPORT_JOURNAL") == 0) {
        return IMPORT_JOURNAL;
    }

    if(strcmp(type_str, "MODIFY_JOURNAL") == 0) {
        return MODIFY_JOURNAL;
    }

    if(strcmp(type_str, "DELETE_JOURNAL") == 0) {
        return DELETE_JOURNAL;
    }

    if(strcmp(type_str, "DISCONNECT_CLIENT") == 0) {
        return DISCONNECT_CLIENT;
    }

    return INVALID_COMMAND;
}

// Change message_length to content_length
message_header* parse_header(char* partial_message) {
    int message_length, user_id;
    char command_type[18];

    int res = sscanf(partial_message, "Header\ncommand-type<::::>%18s\nmessage-length<::::>%d\nuser-id<::::>%d\n", 
        command_type, &message_length, &user_id);
    
    if(res < 2) {
        return NULL;
    }

    message_header* header = malloc(sizeof(message_header));
    header->type = get_command_type(command_type);
    header->length = message_length;
    header->client_id = user_id;

    return header;
}

// Change tags to coresponding tags
message_content* parse_content(char* content_str, size_t content_size) {
    char start_tag[] = "<journal_request_key>";
    char end_tag[] = "</journal_request_key>\n";

    char *target = NULL;
    char *current_pos = content_str;
    void *start, *end;

    size_t current_content_size = content_size;

    message_content* content = (message_content*)calloc(1, sizeof(message_content));
    message_content_node* last = NULL;

    while(1) {
        char* key_end = strstr(current_pos, "=");
        if(!key_end) {
            break;
        }

        *key_end = '\0';
        current_content_size -= strlen(current_pos); 

        if (start = strstr(key_end + 1, start_tag)) {
            start += 22;
            current_content_size -= 22;

            if (end = memmem((void*)start, current_content_size, (void*)end_tag, 24)) {
                target = malloc((size_t)(end - start + 1));
                memcpy(target, start, end - start);
                target[end - start] = '\0';
            }
        }

        if(!target) {
            break;
        }

        message_content_node* node = (message_content_node*)calloc(1, sizeof(message_content_node));
        node->key = (char*)malloc(strlen(key_end));
        strcpy(node->key, current_pos);

        node->node_data= (message_content_node_data*)calloc(1, sizeof(message_content_node_data));
        node->node_data->size = (size_t)(end - start);
        node->node_data->value = (char*)malloc(end - start);
        memcpy(node->node_data->value, target, node->node_data->size);
        current_content_size -= node->node_data->size - 27;

        if(!content->head) {
            content->head = node;
        }
        else {
            last->next = node;
        }

        last = node;
        current_pos = end + 27;
        free(target);
    }

    return content;
}


message_t* parse_message(int socket_fd, char* message_str) {
    message_header* header = parse_header(message_str);
    if(header == NULL) {
        return NULL;
    }

    srandom(time(NULL));
    message_t* message = (message_t*)malloc(sizeof(message));
    message->id = socket_fd * 1000 + (random() % 1000);
    message->header = header;
    message->content = NULL;

    char* content_str = strstr("Content\n", message_str);
    if(content_str) {
        *(content_str) = '\0';
        size_t content_length = header->length - strlen(message_str) - 8;
        message->content = parse_content(content_str + 9, content_length);
    }

    return message;
}


void delete_message(message_t* message) {
    free(message->header);

    if(message->content) {
        message_content_node* node = message->content->head;
        while(node) {
            free(node->key);
            free(node->node_data->value);

            message_content_node* next = node->next;
            free(node);
            node = next;
        }
    }
    
    free(message);
}


message_content* create_message_content(char* keys[], size_t keys_len, char* values[], size_t values_len) {
    if(keys_len == 0 || values == 0 || keys_len != values_len) {
        return (message_content*)NULL;
    }

    message_content* content = (message_content*)malloc(sizeof(message_content));
    message_content_node* last = NULL;
    for(int i = 0; i < keys_len; i++) {
        message_content_node* node = (message_content_node*)malloc(sizeof(message_content_node));
        node->key = (char*)malloc(strlen(keys[i]));
        strcpy(node->key, keys[i]);

        node->node_data->size = strlen(values[i]);
        node->next = NULL;

        node->node_data = (message_content_node_data*)malloc(sizeof(message_content_node_data));
        node->node_data->value = malloc(node->node_data->size);
        memcpy(node->node_data->value, values[i], node->node_data->size);

        if(!content->head) {
            content->head = node;
        }
        else {
            last->next = node;
        }

        last = node;
    }
}
