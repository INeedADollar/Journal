#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#include "message.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


message_header* parse_header(char* partial_message) {
    int message_type, message_length, user_id;

    int res = sscanf(partial_message, "Header\ncommand-type<::::>%d\nmessage-length<::::>%d\nuser-id<::::>%d\n", 
        &message_type, &message_length, &user_id);
    
    if(res < 2) {
        return NULL;
    }

    message_header* header = malloc(sizeof(message_header));
    header->type = message_type;
    header->length = message_length;
    header->id = user_id;

    return header;
}


// de reparat functia cu size ul
message_content* parse_content(char* content_str) {
    char start_tag[] = "<;23sad32fefs.>";
    char end_tag[] = "</sasadasfds.32.asd2qasd>\n";

    char *target = NULL;
    char *current_pos = content_str;
    void *start, *end;

    message_content* content = (message_content*)calloc(1, sizeof(message_content));
    message_content_node* last = NULL;

    while(1) {
        char* key_end = strstr(current_pos, "=");
        if(!key_end) {
            break;
        }

        *key_end = "\0";

        if (start = memmem(key_end + 1, 16, start_tag, 16)) {
            start += 16;
            if (end = memmem((void*)start, (void*)end_tag)) {
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

        node->node_data->size = (size_t)(end - start);
        node->value = (char*)malloc(end - start);
        memcpy(node->node_data->value, target, node->node_data->size);

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


message* parse_message(int socket_fd, char* message_str) {
    message_header* header = parse_header(message_str);
    if(header == NULL) {
        return NULL;
    }

    srandom(time(NULL));
    message* message = (message*)malloc(sizeof(message_str));
    message->id = socket_fd * 1000 + (random() % 1000);
    message->header = header;
    message->content = NULL;

    char* content_str = strstr("Content\n", message_str);
    if(!content_str) {
        delete_message(message);
        return NULL;
    }

    message->content = parse_content(content_str + 9);
    return message;
}


void delete_message(message* message) {
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

        node->value = (message_content_node_value*)malloc(sizeof(message_content_node_value));
        node->value->node_value = malloc(strlen(values[i]));
        memcpy(node->value->node_value, values[i]);

        node->value->size = strlen(values[i]);
        node->next = NULL;

        if(!content->head) {
            content->head = node;
        }
        else {
            last->next = node;
        }

        last = node;
    }
}
