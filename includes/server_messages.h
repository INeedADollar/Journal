#ifndef SERVER_MESSAGES_H
#define SERVER_MESSAGES_H

typedef long int ID;

typedef enum message_type {
    CREATE,
    RETRIEVE,
    IMPORT,
    MODIFY,
    DELETE,
    DISCONNECT
} MessageType;

typedef struct Header {
    int message_length;
    MessageType message_type;
} ServerMessageHeader;

typedef char* Content;

typedef struct Message {
    ID id;
    Header* header;
    Content* content;
} ServerMessage;

Header* parse_header(char* partial_message);

ServerMessage* parse_message(int socked_fd, char* message);

#endif // SERVER_MESSAGES_H