#ifndef SERVER_MESSAGES_H
#define SERVER_MESSAGES_H

typedef long int MESSAGE_ID;
typedef unsigned long USER_ID;
typedef int MESSAGE_LENGTH;

typedef enum {
    GENERATE_ID,
    CREATE_JOURNAL,
    RETRIEVE_JOURNAL,
    IMPORT_JOURNAL,
    MODIFY_JOURNAL,
    DELETE_JOURNAL,
    DISCONNECT_CLIENT
} MESSAGE_TYPES;

typedef struct {
    MESSAGE_TYPES message_type;
    MESSAGE_LENGTH message_length;
    USER_ID user_id;
} MESSAGE_HEADER;

typedef char* MESSAGE_CONTENT_NODE_KEY;

typedef struct {
    void* node_value;
    size_t size;
} MESSAGE_CONTENT_NODE_VALUE;

typedef struct {
    MESSAGE_CONTENT_NODE_KEY key;
    MESSAGE_CONTENT_NODE_VALUE* value;
    MESSAGE_CONTENT_NODE* next;
} MESSAGE_CONTENT_NODE;

typedef struct {
    MESSAGE_CONTENT_NODE* head;
} MESSAGE_CONTENT;

typedef struct {
    MESSAGE_ID id;
    MESSAGE_HEADER* header;
    MESSAGE_CONTENT* content;
} MESSAGE;

MESSAGE_HEADER* parse_header(char* partial_message);

MESSAGE* parse_message(int socked_fd, char* message);

void delete_message(MESSAGE* message);

MESSAGE_CONTENT* create_message_content(char* keys[], char* values[]);

#endif // SERVER_MESSAGES_H