//gsoap_commands.h

#import "../includes/commands.h"
#import "../includes/message.h"


struct create_journal_input {
    user_id id;
    char* journal_name;
};

struct retrieve_journal_input {
    user_id id;
    char* journal_name;
};

struct import_journal_input {
    user_id id;
    char* journal_name;
    char* journal_data;
    size_t journal_data_size;
};

struct modify_journal_input {
    user_id id;
    char* journal_name;
    char* new_content;
    size_t new_content_size;
};

struct delete_journal_input {
    user_id id;
    char* journal_name;
};


int ns__generate_id(struct soap* soap, struct command_result* response);

int ns__create_journal(struct soap* soap, struct create_journal_input* request, command_result* response);

int ns__retrieve_journal(struct soap* soap, struct retrieve_journal_input* request, command_result* response);

int ns__import_journal(struct soap* soap, struct import_journal_input* request, command_result* response);

int ns__modify_journal(struct soap* soap, struct modify_journal_input* request, command_result* response);

int ns__delete_journal(struct soap* soap, struct delete_journal_input* request, command_result* response);
