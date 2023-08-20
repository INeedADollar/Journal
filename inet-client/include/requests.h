#ifndef REQUESTS_H
#define REQUESTS_H


typedef enum {
    FAIL,
    SUCCESS
} status_t;

typedef struct {
    status_t status;
    char* status_message;
    char* data;
    size_t data_size;
} response;

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
} request_type;

typedef void (*notification_callback)(response*, request_type type);


void init_requests(char* server_address, int port);

void register_notifications_callback(notification_callback callback);

response* create_journal(char* journal_name);

response* retrieve_journal(char* journal_name);

response* retrieve_journals();

response* delete_journal(char* journal_name);

int import_journal(char* journal_name, char* journal_content, size_t journal_content_size);

int modify_journal(char* journal_name, char* modified_pages, size_t modified_pages_size);

void disconnect_client();

void delete_response(response* resp);


#endif // REQUESTS_H