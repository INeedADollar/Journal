#include "commands.h"
#include "logger.h"
#include "server_messages.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zip/src/zip.h>
#include <errno.h>
#include <regex.h>
#include <sys/stat.h>


#define CHECK_FOR_FAIL_AND_SEND_MESSAGE(variable, message_error, type, console_error, ...) \
    if(!variable || variable == OPERATION_FAIL) { \
        LOG_ERROR(console_error, __VA_ARGS__, strerror(errno)); \
        send_status_message(message_error, type, OPERATION_FAIL); \
        return OPERATION_FAIL; \
    } \


OPERATION_STATUS send_response(MESSAGE* response) {
    OPERATION_STATUS status = send_message(response);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Error processing your request.", response->header->message_type, 
        "Response for message with type %d could not be sent. Error: %s", response->header->message_type)
    delete_message(response);
    return status;
}


void send_status_message(USER_ID id, char* message_str, MESSAGE_TYPES type, OPERATION_STATUS status) {
    char* keys[] = {
        "status",
        "message"
    };

    char status_str[1];
    itoa((int)status, status_str, 10);

    char* values[] = {
        status_str,
        message_str
    };

    MESSAGE* response_message = (MESSAGE*)calloc(sizeof(MESSAGE));
    response_message->header = (MESSAGE_HEADER*)malloc(sizeof(MESSAGE_HEADER));
    response_message->header->message_type = type;
    response_message->header->user_id = id;
    
    MESSAGE_CONTENT* content = create_message_content(keys, 2, values, 2);
    response_message->content = content;

    send_response(response_message);
}


MESSAGE_CONTENT_NODE_VALUE* extract_value_from_content(MESSAGE_CONTENT* content, char* key) {
    if(!content) {
        return NULL;
    }

    MESSAGE_CONTENT_NODE* node = content->head;
    while(node) {
        if(strcmp(node->key, key) == 0) {
            return node;
        }
    }

    return NULL;
}


OPERATION_STATUS generate_id(MESSAGE* message) {
    char message[50];
    USER_ID new_id = (USER_ID)time(NULL) + (random() % 1000);

    sprintf(message, "Generated id is %lu", new_id);
    send_status_message(new_id, message, GENERATE_ID, OPERATION_SUCCESS);

    return OPERATION_SUCCESS;
}


OPERATION_STATUS create_journal(MESSAGE* message){
    char response_content[512]; 
    char journal_path[512];
    char dirname[512];
    char* zip_files[] = {
        "1.txt"
    };

    USER_ID user_id;

    MESSAGE_CONTENT_NODE_VALUE* journal_name = extract_value_from_content(message->content, "journal-name");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message,
        "Journal name %s is invalid for user with id %lu. Error: %s\n", journal_name, message->header->user_id);
    LOG_DEBUG("Journal name %s is present and a valid name.", journal_name);

    sprintf(dirname, "%lu", user_id);
    OPERATION_STATUS status = (OPERATION_STATUS)mkdir(dirname, 0777);
    status = (status == OPERATION_FAIL && errno == EEXIST) || (status == OPERATION_SUCCESS);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be created.", message,
        "Directory %s could not be created for user with id %lu. Error: %s\n", dirname, message->header->user_id);
    LOG_INFO("Created directory %s for client with user id %lu\n", dirname, user_id);

    sprintf(journal_path, "%s/%s.zip", dirname, journal_name->node_value);
    status = (OPERATION_STATUS)zip_create(dirname, zip_files, 1)
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be created.", message, 
        "Zip %s could not be created for user with id %lu. Error: %s\n", journal_path, message->header->user_id);

    LOG_DEBUG("Journal %s created succesfully.\n", journal_name)
    send_status_message("Journal created successfully.", CREATE_JOURNAL, OPERATION_SUCCESS);
    return OPERATION_SUCCESS;
}


OPERATION_STATUS retrieve_journal(MESSAGE* message) {
    MESSAGE_CONTENT_NODE_VALUE* journal_name = extract_value_from_content(message->content, "journal-name");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message,
        "Journal name %s is invalid for user with id %lu. Error: %s\n", journal_name, message->header->user_id);

    char journal_path[1024];
    sprintf(journal_path, "./journals/%d/%s.zip", message->header->user_id, journal_name);

    int journal_fd = open(journal_path, O_RDONLY);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_fd, "Journal could not be found on the server.", message, content, 
        "The journal %s could not be found! Error: %s\n", journal_path);

    struct stat st;
    stat(journal_fd, &st);
    
    char content[st.st_size];
    OPERATION_STATUS status = (OPERATION_STATUS)read(journal_fd, content, st.st_size);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be sent.", message, content, 
        "The journal %s could not be read! Error: %s\n", journal_path);

    close(journal_fd);

    char* keys[] = {
        "status",
        "journal"
    };

    char* values[] = {
        "1",
        content
    };

    MESSAGE* response_message = (MESSAGE*)malloc(sizeof(MESSAGE));
    response_message->header = (MESSAGE_HEADER*)malloc(sizeof(MESSAGE_HEADER));
    response_message->header->message_type = RETRIEVE_JOURNAL;
    
    MESSAGE_CONTENT* content = create_message_content(keys, 2, values, 2);
    response_message->content = content;

    create_and_send_response(message);
    delete_message(message);
    return OPERATION_SUCCESS;
}


OPERATION_STATUS import_journal(MESSAGE* message) {
    MESSAGE_CONTENT_NODE_VALUE* journal_name = extract_value_from_content(message->content, "journal-name");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message,
        "Journal name %s is invalid for user with id %lu. Error: %s\n", journal_name, message->header->user_id);

    char journal_path[1024];
    sprintf(journal_path, "./journals/%d/%s.zip", message->header->user_id, journal_name);
    
    int journal_fd = open(journal_path, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_fd, "Journal could not be imported.", message, content, 
        "The journal %s could not be created before import! Error: %s\n", journal_path);
    
    MESSAGE_CONTENT_NODE_VALUE* journal_data = extract_value_from_content(message->content, "journal-data");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_data, "Journal could not be imported. Check the file and try again.", message, content, 
        "The journal %s could not be imported because content of it is not present! Error: %s\n", journal_path);

    OPERATION_STATUS status = write(journal_fd, journal_data->node_value, journal_data->size);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be imported.", message, content, 
        "The journal %s could not be imported because content of it cannot be written! Error: %s\n", journal_path);
    
    close(journal_fd);

    LOG_DEBUG("Journal %s imported succesfully.\n", journal_name)
    send_status_message("Journal imported successfully.", IMPORT_JOURNAL, OPERATION_SUCCESS);
    return OPERATION_SUCCESS;
}


OPERATION_STATUS modify_journal(MESSAGE* message) {
    MESSAGE_CONTENT_NODE_VALUE* journal_name = extract_value_from_content(message->content, "journal-name");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message,
        "Journal name %s is invalid for user with id %lu. Error: %s\n", journal_name, message->header->user_id);

    char journal_path[1024];
    sprintf(journal_path, "./journals/%d/%s.zip", message->header->user_id, journal_name);
    struct zip_t zip = zip_open(journal_path, ZIP_DEFAULT_COMPRESSION_LEVEL, "r");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(zip, "Journal could not be found on the server.", message, content, 
        "The journal %s could not be found! Error: %s\n", journal_path);

    char* content = NULL;
    int total_content_size = 0;
    struct zip_t *zip = zip_open("foo.zip", 0, 'r');

    int total_entries = zip_entries_total(zip);
    for(int i = 0; i < total_entries; i++) {
        if(zip_entry_isdir(zip)) {
            continue;
        }

        zip_entry_openbyindex(zip, i);

        unsigned long long size = zip_entry_size(zip);
        char entry_content[size];
        size_t entry_content_size;

        zip_entry_read(zip, &entry_content, &entry_content_size);
        if(!content) {
            content = (char*)malloc(entry_content_size);
        }
        else {
            content = (char*)realloc(content, total_content_size + entry_content_size);
        }

        strcpy(content + total_content_size, entry_content);


        zip_entry_close(zip);
        free(buf);
    }
    zip_entry_open(zip, ".txt");
    


    zip_close(zip);

 
    sprintf(content, "status=%d\nmessage=%s\n", OPERATION_SUCCESS, "Journal created successfully.");
    create_and_send_response(message, content);
}


OPERATION_STATUS delete_journal(MESSAGE_HEADER* message_header, char* message_part, int socket_fd){
    char read_message[1024];
    char line[512];
    char content[512];
    USER_ID user_id;

    free(message_header);
    read_all_message(socket_fd, read_message, message_part);

    MESSAGE* message = parse_message(socket_fd, read_message);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(message, "Recieved message is invalid.", message, content);

    MESSAGE_CONTENT_NODE_VALUE* journal_name = extract_value_from_content(message->content, "journal-name");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message,
        "Journal name %s is invalid for user with id %lu. Error: %s\n", journal_name, message->header->user_id);

    char journal_path[1024];
    sprintf(journal_path, "./journals/%d/%s.zip", message->header->user_id, journal_name);
    OPERATION_STATUS status = (OPERATION_STATUS)remove(journal_path);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be deleted.", message, content, 
        "The journal %s could not be deleted! Error: %s\n", journal_path);

    printf("The journal was deleted successfully!\n");
    char* keys[] = {
        "status",
        "message"
    };

    char* values[] = {
        "1",
        "Journal deleted successfully."
    };

    MESSAGE* response_message = (MESSAGE*)malloc(sizeof(MESSAGE));
    response_message->header = (MESSAGE_HEADER*)malloc(sizeof(MESSAGE_HEADER));
    response_message->header->message_type = CREATE_JOURNAL;
    
    MESSAGE_CONTENT* content = create_message_content(keys, 2, values, 2);
    response_message->content = content;

    create_and_send_response(message);
    delete_message(message);
    return OPERATION_SUCCESS;
}


OPERATION_STATUS disconnect_client(MESSAGE_HEADER* header, int sochet_fd){
    printf("Client %lu has disconnected!\n", header->user_id);
    free(header);
    return (OPERATION_STATUS)close(sochet_fd);
}