#include "commands.h"
#include "logger.h"
#include "message.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zip/src/zip.h>
#include <errno.h>
#include <regex.h>
#include <sys/stat.h>


operation_status generate_id(message* message) {
    char message[50];
    user_id new_id = (user_id)time(NULL) + (random() % 1000);

    sprintf(message, "Generated id is %lu", new_id);
    send_status_message(new_id, message, GENERATE_ID, OPERATION_SUCCESS);

    return OPERATION_SUCCESS;
}


operation_status create_journal(message* message){
    char response_content[512]; 
    char journal_path[512];
    char dirname[512];
    char* zip_files[] = {
        "1.txt"
    };

    user_id user_id;

    MESSAGE_CONTENT_NODE_VALUE* journal_name = extract_value_from_content(message->content, "journal-name");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message,
        "Journal name %s is invalid for user with id %lu. Error: %s\n", journal_name, message->header->user_id);
    LOG_DEBUG("Journal name %s is present and a valid name.", journal_name);

    sprintf(dirname, "%lu", user_id);
    operation_status status = (operation_status)mkdir(dirname, 0777);
    status = (status == OPERATION_FAIL && errno == EEXIST) || (status == OPERATION_SUCCESS);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be created.", message,
        "Directory %s could not be created for user with id %lu. Error: %s\n", dirname, message->header->user_id);
    LOG_INFO("Created directory %s for client with user id %lu\n", dirname, user_id);

    sprintf(journal_path, "%s/%s.zip", dirname, journal_name->node_value);
    status = (operation_status)zip_create(dirname, zip_files, 1)
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be created.", message, 
        "Zip %s could not be created for user with id %lu. Error: %s\n", journal_path, message->header->user_id);

    LOG_DEBUG("Journal %s created succesfully.\n", journal_name)
    send_status_message("Journal created successfully.", CREATE_JOURNAL, OPERATION_SUCCESS);
    return OPERATION_SUCCESS;
}


operation_status retrieve_journal(message* message) {
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
    operation_status status = (operation_status)read(journal_fd, content, st.st_size);
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

    message* response_message = (message*)malloc(sizeof(message));
    response_message->header = (MESSAGE_HEADER*)malloc(sizeof(MESSAGE_HEADER));
    response_message->header->message_type = RETRIEVE_JOURNAL;
    
    MESSAGE_CONTENT* content = create_message_content(keys, 2, values, 2);
    response_message->content = content;

    create_and_send_response(message);
    delete_message(message);
    return OPERATION_SUCCESS;
}


operation_status import_journal(message* message) {
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

    operation_status status = write(journal_fd, journal_data->node_value, journal_data->size);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be imported.", message, content, 
        "The journal %s could not be imported because content of it cannot be written! Error: %s\n", journal_path);
    
    close(journal_fd);

    LOG_DEBUG("Journal %s imported succesfully.\n", journal_name);
    send_status_message("Journal imported successfully.", IMPORT_JOURNAL, OPERATION_SUCCESS);
    return OPERATION_SUCCESS;
}


operation_status modify_journal(message* message) {
    MESSAGE_CONTENT_NODE_VALUE* journal_name = extract_value_from_content(message->content, "journal-name");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message,
        "Journal name %s is invalid for user with id %lu. Error: %s\n", journal_name, message->header->user_id);

    MESSAGE_CONTENT_NODE_VALUE* new_content = extract_value_from_content(message->content, "new-content");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "New content is invalid or missing.", message,
        "Journal content is invalid or missing for user with id %lu. Error: %s\n", message->header->user_id);
    
    char journal_path[1024];
    sprintf(journal_path, "./journals/%d/%s.zip", message->header->user_id, journal_name);
    struct zip_t* journal_zip = zip_open(journal_path, ZIP_DEFAULT_COMPRESSION_LEVEL, "w");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_zip, "Journal could not be found on the server.", message, content, 
        "The journal %s could not be found! Error: %s\n", journal_path);

    struct zip_t *new_content_zip = zip_stream_open(new_content->node_value, new_content->size, ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(new_content_zip, "Sent journal could not be opened.", message, content, 
        "Sent journal could not be opened! Error: %s\n");

    char* content = NULL;
    int total_entries = zip_entries_total(new_content_zip);

    for(int i = 0; i < total_entries; i++) {
        operation_status status = zip_entry_openbyindex(new_content_zip, i);
        CHECK_FOR_FAIL_AND_SEND_MESSAGE(new_content_zip, "Failed to import journal.", message, content, 
            "Failed to open new content zip entry %d! Error: %s\n", i);
        
        if(zip_entry_isdir(new_content_zip)) {
            LOG_WARNING("New content zip entry %d is a folder!", i)
            zip_entry_close(new_content_zip);
            continue;
        }

        const char* entry_name = zip_entry_name(new_content_zip);
        CHECK_FOR_FAIL_AND_SEND_MESSAGE(entry_name, "Failed to import journal.", message, content, 
            "Failed to get new content zip entry %d name! Error: %s\n", i);

        status = zip_entry_open(journal_zip, entry_name);
        CHECK_FOR_FAIL_AND_SEND_MESSAGE(new_content_zip, "Failed to import journal.", message, content, 
            "Failed to create journal zip entry %s! Error: %s\n", entry_name);

        char* entry_data = NULL;
        size_t entry_data_size = 0;
        zip_entry_read(new_content_zip, (void**)&entry_data, &entry_data_size);
        zip_entry_write(journal_zip, (void*)entry_data, entry_data_size);

        zip_entry_close(journal_zip);
        zip_entry_close(new_content_zip);
        free(entry_data);
    }

    zip_close(journal_zip);
    zip_close(new_content_zip);

    send_status_message("Journal modified successfully.", MODIFY_JOURNAL, OPERATION_SUCCESS);
    return OPERATION_SUCCESS;
}


operation_status delete_journal(MESSAGE_HEADER* message_header, char* message_part, int socket_fd){
    MESSAGE_CONTENT_NODE_VALUE* journal_name = extract_value_from_content(message->content, "journal-name");
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(journal_name, "Journal name is invalid or missing.", message,
        "Journal name %s is invalid for user with id %lu. Error: %s\n", journal_name, message->header->user_id);

    char journal_path[1024];
    sprintf(journal_path, "./journals/%d/%s.zip", message->header->user_id, journal_name);
    operation_status status = (operation_status)remove(journal_path);
    CHECK_FOR_FAIL_AND_SEND_MESSAGE(status, "Journal could not be deleted.", message, content, 
        "Journal %s could not be deleted! Error: %s\n", journal_path);

    LOG_INFO("The journal was deleted successfully!");
    send_status_message("Journal deleted successfully.", MODIFY_JOURNAL, OPERATION_SUCCESS);
    return OPERATION_SUCCESS;
}


operation_status disconnect_client(message message){
    int socket_fd = message->id / 1000;
    int user_id = message->header->user_id;
    delete_message(message);

    LOG_INFO("Client %lu has disconnected!\n", user_id);
    return (operation_status)close(socket_fd);
}