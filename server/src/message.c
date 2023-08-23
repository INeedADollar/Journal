#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#include "message.h"
#include "logger.h"

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


message_header* parse_header(char* partial_message) {
    size_t content_length, user_id;
    char command_type[18];

    int res = sscanf(partial_message, "Header\ncommand-type<::::>%18s\ncontent-length<::::>%zu\nuser-id<::::>%lu\n", 
        command_type, &content_length, &user_id);
    
    if(res < 2) {
        return NULL;
    }

    message_header* header = malloc(sizeof(message_header));
    header->type = get_command_type(command_type);
    header->length = content_length;
    header->client_id = user_id;

    return header;
}


message_content* parse_content(char* content_str, size_t content_size) {
    char start_tag[] = "<journal_request_value>";
    char end_tag[] = "</journal_request_value>";

    char *target = NULL;
    char *current_pos = content_str;
    void *start, *end;

    size_t current_content_size = content_size;

    message_content* content = (message_content*)calloc(1, sizeof(message_content));
    message_content_node* last = NULL;

    while(1) {
        log_info(current_pos);
        char* key_end = strstr(current_pos, "=");
        if(!key_end) {
            break;
        }

        *key_end = '\0';
        current_content_size -= strlen(current_pos); 
        log_info("Current pos %s", current_pos);

        if (start = strstr(key_end + 1, start_tag)) {
            start += 23;
            current_content_size -= 23;

            log_info("START");
            if (end = memmem((void*)start, current_content_size, (void*)end_tag, 24)) {
                target = malloc((size_t)(end - start + 1));
                memcpy(target, start, end - start);
                target[end - start] = '\0';
            }
        }

        if(!target) {
            log_info("BREAK");
            break;
        }

        message_content_node* node = (message_content_node*)calloc(1, sizeof(message_content_node));
        node->key = (char*)malloc(strlen(current_pos) + 1);
        strcpy(node->key, current_pos);
        
        log_info("Node key %s %s", node->key, current_pos);

        node->node_data = (message_content_node_data*)calloc(1, sizeof(message_content_node_data));
        node->node_data->size = (size_t)(end - start);
        node->node_data->value = (char*)target;
        current_content_size -= node->node_data->size - 27;

        if(!content->head) {
            content->head = node;
        }
        else {
            last->next = node;
        }

        last = node;
        current_pos = end + 25;
        target = NULL;
    }

    if(last) {
        last->next = NULL;
    }

    return content;
}


message_header* get_message_header(char* message_str, message_header* header) {
    if(header) {
        return header;
    }

    header = parse_header(message_str);
    return header ? header : NULL;
}


message_t* parse_message(int socket_fd, char* message_str, message_header* header) {
    log_info("PARSE");
    message_header* new_header = get_message_header(message_str, header);
    if(!new_header) {
        log_info("NO HEADER");
        return NULL;
    }

    log_info("%zu", new_header->length);
    srandom(time(NULL));
    message_t* message = (message_t*)malloc(sizeof(message));
    message->id = socket_fd * 1000 + (random() % 1000);
    message->header = new_header;
    message->content = NULL;

    char* content_str = NULL;
    if(!header) {
        content_str = strstr(message_str, "Content\n");
        log_info(message_str);
        if(content_str && new_header->length > 0) {
            message->content = parse_content(content_str + 8, new_header->length);
        }
    }
    else {
        message->content = parse_content(message_str, header->length);
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
