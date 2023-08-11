#ifndef MESSAGES_H
#define MESSAGES_H

typedef long int message_id;
typedef unsigned long user_id;
typedef int message_length;

typedef enum {
    GENERATE_ID,
    CREATE_JOURNAL,
    RETRIEVE_JOURNAL,
    IMPORT_JOURNAL,
    MODIFY_JOURNAL,
    DELETE_JOURNAL,
    DISCONNECT_CLIENT
} message_types;

typedef struct {
    message_types type;
    message_length length;
    user_id client_id;
} message_header;

typedef char* message_content_node_key;

typedef struct {
    char* value;
    size_t size;
} message_content_node_data;

typedef struct {
    message_content_node_key key;
    message_content_node_data* node_data;
    message_content_node* next;
} message_content_node;

typedef struct {
    message_content_node* head;
} message_content;

typedef struct {
    message_id id;
    message_header* header;
    message_content* content;
} message;

message_header* parse_header(char* partial_message);

message* parse_message(int socked_fd, char* message);

void delete_message(message* message);

message_content* create_message_content(char* keys[], char* values[]);

#endif // MESSAGES_H