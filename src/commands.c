#include "commands.h"
#include "server_messages.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zip/src/zip.h>
#include <errno.h>
#include <regex.h>

#define CHECK_FOR_FAIL_AND_SEND_MESSAGE(variable, error, message, content, console_error, ...) \
    if(!variable || variable == OPERATION_FAIL) { \
        fprintf(stderr, console_error, __VA_ARGS__, strerror(errno)); \
        sprintf(content, "status=%d\nmessage=%s\n", OPERATION_FAIL, error); \
        create_and_send_response(message, content); \
        return OPERATION_FAIL; \
    } \

void create_and_send_response(MESSAGE* message, char* content) {
    message->header->message_length = strlen(content);
    message->content = content;

    send_message(message);
    delete_message(message);
}


void read_all_message(int socket_fd, char* message, char* read_part) {
    char line[512];
    size_t received = recv(socket_fd, &line, 512, 0);
    line[received] = "\0";

    sprintf(message, "%s%s", read_part, line);
}

OPERATION_STATUS generate_id(MESSAGE_HEADER* header, int socket_fd){
    char content[100];
    free(header);

    USER_ID new_id = time(NULL) + (random() % 1000); 
    sprintf(content, "new-id=%lu", new_id);
    printf("Created id=%lu for client %d\n", new_id, socket_fd);

    MESSAGE* message = (MESSAGE*)malloc(sizeof(MESSAGE));
    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)malloc(sizeof(MESSAGE_HEADER))
    message_header->message_type = GENERATE_ID;
    message->user_id = new_id;

    message->header = message_header;

    create_and_send_response(message, content);
    return OPERATION_SUCCESS;
}


OPERATION_STATUS create_journal(MESSAGE_HEADER* message_header, char* message_part, int socket_fd){
    char read_message[1024];
    char response_content[512]; 
    USER_ID user_id;

    free(message_header);
    read_all_message(socket_fd, read_message, message_part);

    MESSAGE* message = parse_message(socket_fd, read_message);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(message, "Recieved message is invalid.", message, content);

    char dirname[512];
    sprintf(dirname, "%lu", user_id);
    OPERATION_STATUS status = (OPERATION_STATUS)mkdir(dirname, 0777);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be created.", message, content);

    printf("Created directory %s for client with user id %lu\n", dirname, user_id);
    char* journal_name = strstr(message->content, "=");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message, content);

    char journal_path[512];
    char* zip_files[] = {
        "1.txt"
    }

    sprintf(journal_path, "%s/%s.zip", dirname, journal_name);
    status = (OPERATION_STATUS)zip_create(dirname, zip_files, 1)
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be created.", message, content);

    sprintf(content, "status=%d\nmessage=%s\n", OPERATION_SUCCESS, "Journal created successfully.");
    create_and_send_response(message, content);
    return OPERATION_SUCCESS;
}


OPERATION_STATUS retrieve_journal(MESSAGE* message) {
    char* journal_name = strstr(message->content, "=");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message, content);

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


OPERATION_STATUS import_journal(MESSAGE* message) {
        char* journal_name = strstr(message->content, "=");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message, content);

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


OPERATION_STATUS modify_journal(MESSAGE* message) {
        char* journal_name = strstr(message->content, "=");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message, content);

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

    char* journal_name = strstr(message->content, "=");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message, content);

    char journal_path[1024];
    sprintf(journal_path, "./journals/%d/%s.zip", message->header->user_id, journal_name);
    OPERATION_STATUS status = (OPERATION_STATUS)remove(journal_path);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be deleted.", message, content, 
        "The journal %s could not be deleted! Error: %s\n", journal_path);

    printf("The journal was deleted successfully!\n");
    sprintf(content, "status=%d\nmessage=%s\n", OPERATION_SUCCESS, "Journal deleted successfully.");
    create_and_send_response(message, content);
    return OPERATION_SUCCESS;
}


OPERATION_STATUS disconnect_client(MESSAGE_HEADER* header, int sochet_fd){
    printf("Client %lu has disconnected!\n", header->user_id);
    free(header);
    return (OPERATION_STATUS)close(sochet_fd);
}