#ifndef SERVER_DEFS_H
#define SERVER_DEFS_H

#include <stddef.h>


typedef enum {
    GENERATE_ID,
    CREATE_JOURNAL,
    RETRIEVE_JOURNAL,
    RETRIEVE_JOURNALS,
    IMPORT_JOURNAL,
    MODIFY_JOURNAL,
    DELETE_JOURNAL,
    DISCONNECT_CLIENT,
    INVALID_COMMAND
} command_types;

typedef enum {
    OPERATION_SUCCESS = 0,
    OPERATION_FAIL = -1
} operation_status;

typedef struct {
    command_types type;
    operation_status status;
    char status_message[1000];
    char* additional_data;
    size_t additional_data_size;
} command_result;


static int STOP_SERVER = 0;


typedef long int message_id;
typedef unsigned long user_id;
typedef size_t message_length;

typedef struct {
    command_types type;
    message_length length;
    user_id client_id;
} message_header;

typedef char* message_content_node_key;

typedef struct {
    char* value;
    size_t size;
} message_content_node_data;

typedef struct content_node {
    message_content_node_key key;
    message_content_node_data* node_data;
    struct content_node* next;
} message_content_node;

typedef struct {
    message_content_node* head;
} message_content;

typedef struct {
    message_id id;
    message_header* header;
    message_content* content;
} message_t;


typedef struct {
    user_id id;
    char* journal_name;
} retrieve_journal_args;

typedef struct {
    user_id id;
    size_t journal_content_size;
    char* journal_name;
    char* journal_content; 
} import_journal_args;

typedef struct {
    user_id id;
    size_t new_content_size;
    char* journal_name;
    char* new_content;
} modify_journal_args;


typedef struct {
    message_t* messages[1000];
    int length;
} messages_queue;


#endif // SERVER_DEFS_H