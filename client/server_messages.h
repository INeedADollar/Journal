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

typedef char* MESSAGE_CONTENT;

typedef struct Message {
    MESSAGE_ID id;
    MESSAGE_HEADER* header;
    MESSAGE_CONTENT* content;
} MESSAGE;

MESSAGE_HEADER* parse_header(char* partial_message);

MESSAGE* parse_message(int socked_fd, char* message);

void delete_message(MESSAGE* message);

#endif // SERVER_MESSAGES_H