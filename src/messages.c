#include "server_messages.h"
#include <stdlib.h>
#include <stdio.h>


message_header* parse_header(char* partial_message) {
    int message_type, message_length, user_id;

    int res = sscanf(partial_message, "Header\nmessage-type<::::>%d\nmessage-length<::::>%d\nuser-id<::::>%d\n", 
        &message_type, &message_length, &user_id);
    
    if(res < 2) {
        return NULL;
    }

    message_header* header = malloc(sizeof(message_header));
    header->message_type = message_type;
    header->message_length = message_length;
    header->user_id = user_id;

    return header;
}


message_content* parse_content(char* content_str) {
    char start_tag[] = "<;23sad32fefs.>";
    char end_tag[] = "</sasadasfds.32.asd2qasd>\n";

    char *target = NULL;
    char *current_pos = content_str;
    void *start, *end;

    message_content* content = (message_content*)calloc(sizeof(message_content));
    message_content_node* last = NULL;

    while(1) {
        char* key_end = strstr(current_pos, "=");
        if(!key_end) {
            break;
        }

        *key_end = "\0";

        if (start = memmem(key_end + 1, start_tag)) {
            start += 16;
            if (end = memmem((void*)start, (void*)end_tag)) {
                target = (void*)malloc(end - start + 1);
                memcpy(target, start, end - start);
                target[end - start] = '\0';
            }
        }

        if(!target) {
            break;
        }

        message_content_node* node = (message_content_node*)calloc(sizeof(message_content_node));
        node->key = (char*)malloc(strlen(key_end));
        strcpy(node->key, content);

        node->value = (char*)malloc(end - start);
        memcpy(node->value->node_value, target)
        node->value->size = (size_t)(end - start);

        last = node;
        if(!content->head) {
            content->head = node;
        }
        else {
            last->next = node;
        }

        current_pos = end + 27;
        free(target);
    }

    return content;
}


message* parse_message(int socket_fd, char* message) {
    message_header* header = parse_header(message);
    if(header == NULL) {
        return NULL;
    }

    srandom(time(NULL));
    message* message = malloc(sizeof(message));
    message->id = socket_fd * 1000 + (random() % 1000);
    message->header = header;

    char* content_str = (char*)(message + (strlen(message) - header->message_length));
    message->content = parse_content(content_str);

    return server_message;
}


void delete_message(message* message) {
    free(message->header);

    if(message->content) {
        message_content_node* node = message->content->head;
        while(node) {
            free(node->key);
            free(node->value->node_value);

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
        node->key = (char*)malloc(strlen(keys[i]);
        strcpy(node->key, keys[i]);

        node->value = (MESSAGE_CONTENT_NODE_VALUE*)malloc(sizeof(MESSAGE_CONTENT_NODE_VALUE));
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