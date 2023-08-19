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

typedef void (*notification_callback)(response*);


void init_requests(char* server_address, int port, notification_callback callback);

response* create_journal(char* journal_name);

response* retrieve_journal(char* journal_name);

response* retrieve_journals();

response* import_journal(char* journal_name, char* journal_content, size_t journal_content_size);

response* modify_journal(char* journal_name, char* modified_pages, size_t modified_pages_size);

response* delete_journal(char* journal_name);

void disconnect_client();


#endif // REQUESTS_H