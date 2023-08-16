#define _DEFAULT_SOURCE

#include "commands.h"
#include "async_tasks.h"
#include "logger.h"
#include "message.h"
#include "zip.h"
#include "utils.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h> 


command_result* get_command_result(operation_status status, char* status_message, char* additional_data, size_t additional_data_size) {
    command_result* result = calloc(1, sizeof(command_result));
    result->status = status;

    strcpy(result->status_message, status_message);

    if(additional_data) {
        result->additional_data_size = additional_data_size;
        result->additional_data = (char*)malloc(additional_data_size);
        strcpy(result->additional_data, additional_data);
    }

    return result;
}


command_result* check_journal_name(message_content_node_data* journal_name, user_id id) {
    if(!journal_name) {
        log_error("Request for create journal made by client %lu is missing journal_name key from content!", id);
        return get_command_result(OPERATION_FAIL, "Key journal_name is missing from request content!", NULL, 0);
    }

    return NULL;
}

command_result* check_message_and_run_command(message_t* message) {
    command_result* result = NULL;
    message_content_node_data* journal_name = extract_value_from_content(message->content, "journal-name");
    operation_status op_status;

    switch (message->header->type)
    {
    case GENERATE_ID:
        result = generate_id(message->id / 1000);
        break;
    case CREATE_JOURNAL:
        result = check_journal_name(journal_name, message->header->client_id);
        if(!result) {
            result = create_journal(message->header->client_id, journal_name->value);
        }
        break;
    case RETRIEVE_JOURNAL:
        result = check_journal_name(journal_name, message->header->client_id);
        if(!result) {
            op_status = create_retrieve_journal_task(message->header->client_id, journal_name->value);
            if(op_status == OPERATION_FAIL) {
                log_error("Could not create retrieve journal task for client %lu.", message->header->client_id);
                result = get_command_result(OPERATION_FAIL, "Operation could not be started.", NULL, 0);
            }
        }
        break;
    case RETRIEVE_JOURNALS:
        result = retrieve_journals(message->header->client_id);
    case IMPORT_JOURNAL:
        result = check_journal_name(journal_name, message->header->client_id);
        if(result) {
            break;
        }

        message_content_node_data* journal_data = extract_value_from_content(message->content, "journal-data");
        if(!journal_data) {
            log_error("Request for create journal made by client %lu is missing journal_data key from content!", message->header->client_id);
            result = get_command_result(OPERATION_FAIL, "Key journal_data is missing from request content!", NULL, 0);
            break;
        }
        
        op_status = create_import_journal_task(message->header->client_id, journal_name->value, journal_data->value, journal_data->size);
        if(op_status == OPERATION_FAIL) {
            log_error("Could not create import journal task for client %lu.", message->header->client_id);
            result = get_command_result(OPERATION_FAIL, "Operation could not be started.", NULL, 0);
        }
        break;
    case MODIFY_JOURNAL:
        result = check_journal_name(journal_name, message->header->client_id);
        if(result) {
            break;
        }

        message_content_node_data* new_content = extract_value_from_content(message->content, "new-content");
        if(!new_content) {
            log_error("Request for create journal made by client %lu is missing new_content key from content.", message->header->client_id);
            result = get_command_result(OPERATION_FAIL, "Key journal_data is missing from request content!", NULL, 0);
            break;
        }
        
        op_status = create_modify_journal_task(message->header->client_id, journal_name->value, new_content->value, new_content->size);
        if(op_status == OPERATION_FAIL) {
            log_error("Could not create modify journal task for client %lu.", message->header->client_id);
            result = get_command_result(OPERATION_FAIL, "Operation could not be started.", NULL, 0);
        }
        break;
    case DELETE_JOURNAL:
        result = check_journal_name(journal_name, message->header->client_id);
        if(!result) {
            result = delete_journal(message->header->client_id, journal_name->value);
        }
        break;
    case DISCONNECT_CLIENT:
        disconnect_client(message->header->client_id, message->id / 1000);
    case INVALID_COMMAND:
        result = get_command_result(OPERATION_FAIL, "Command type could not be determined.", NULL, 0);
    default:
        break;
    }

    delete_message(message);
    return result;
}


command_result* generate_id(int client_fd) {
    char* id_string = (char*)malloc(30);

    srand(time(NULL));
    user_id generated_id = (user_id)time(NULL) + (rand() % 1000);
    sprintf(id_string, "%lu", generated_id);

    log_debug("Generated id %lu for client socket %d", generated_id, client_fd);
    return get_command_result(OPERATION_SUCCESS, "Id succesfully generated.", id_string, 30);
}


int get_number_of_user_journals(user_id id) {
    struct dirent* journal;
    char user_directory_path[30];
    sprintf(user_directory_path, "./journals/%lu", id);

    DIR* user_directory = opendir(user_directory_path);
    int journals_count = -1;
    
    if(user_directory) {
        journals_count++;
        while ((journal = readdir(user_directory)) != NULL) {
            if (journal->d_type == DT_REG && string_ends_with(journal->d_name, ".zip")) {
                journals_count++;
            }
        }

        closedir(user_directory);
    }

    return journals_count;
}

command_result* prepare_journal_creation(user_id id) {
    int journals_count = get_number_of_user_journals(id);
    if(journals_count == -1 && errno != ENOENT) {
        return get_command_result(OPERATION_FAIL, "Operation could not be completed.", NULL, 0);
    }

    if(journals_count == 100) {
        return get_command_result(OPERATION_FAIL, "Maximum limit of journals reached.", NULL, 0);
    }

    char dirname[30];
    sprintf(dirname, "journals/%lu", id);
    if(mkdir(dirname, 777) == OPERATION_FAIL && errno != EEXIST) {
        log_error("Directory %s could not be created for user with id %lu. Error: %s", id, strerror(errno));
        return get_command_result(OPERATION_FAIL, "Journal could not be created.", (char*)NULL, 0);
    }

    log_debug("Created directory %s for client with user id %lu", dirname, id);
    return NULL;
}

command_result* create_journal(user_id id, char* journal_name) {
    command_result* result = prepare_journal_creation(id);
    if(result) {
        return result;
    }

    char journal_path[1024];
    const char* zip_files[] = {
        "1.txt"
    };

    sprintf(journal_path, "./journals/%lu/%s.zip", id, journal_name);
    operation_status status = (operation_status)zip_create(journal_path, (const char**)zip_files, 1);
    if(status == OPERATION_FAIL) {
        log_error("Zip %s could not be created for user with id %lu. Error: %s.", id, strerror(errno));
        return get_command_result(OPERATION_FAIL, "Journal could not be created.", (char*)NULL, 0);
    }

    log_debug("Journal %s created succesfully.\n", journal_name);
    return get_command_result(OPERATION_SUCCESS, "Journal created succesfully.", NULL, 0);
}


void get_journal_path(user_id id, char* journal_name, char* buffer) {
    sprintf(buffer, "./journals/%lu/%s.zip", id, journal_name);
}

command_result* retrieve_journal(user_id id, char* journal_name) {
    char journal_path[1024];
    get_journal_path(id, journal_name, journal_path);

    int journal_fd = open(journal_path, O_RDONLY);
    if(journal_fd == OPERATION_FAIL) {
        log_error("Journal %s could not be found! Error: %s", journal_path, strerror(errno));
        return get_command_result(OPERATION_FAIL, "Journal could not be found on the server.", (char*)NULL, 0);
    }

    struct stat st;
    fstat(journal_fd, &st);
    
    char content[st.st_size];
    if(read(journal_fd, content, st.st_size) == OPERATION_FAIL) {
        log_error("Journal %s could not be read! Error: %s", journal_path, strerror(errno));
        return get_command_result(OPERATION_FAIL, "Journal exists but could not be sent.", (char*)NULL, 0);
    }

    close(journal_fd);
    log_debug("Journal %s retrieved succesfully.", journal_path);
    return get_command_result(OPERATION_SUCCESS, "Journal retrieved succesfully.", content, st.st_size);
}


command_result* retrieve_journals(user_id id) {
    struct dirent* journal;
    char user_directory_path[30];
    sprintf(user_directory_path, "./journals/%lu", id);

    DIR* user_directory = opendir(user_directory_path);
    size_t journals_size = 100, current_journals_size = 0;
    char* journals = (char*)malloc(journals_size);

    if(user_directory) {
        while ((journal = readdir(user_directory)) != NULL) {
            if (journal->d_type != DT_REG || !string_ends_with(journal->d_name, ".zip")) {
                continue;
            }

            current_journals_size += strlen(journal->d_name) + 30;
            if(current_journals_size > journals_size) {
                journals_size *= 2;
                journals = (char*)realloc((void*)journals, journals_size);
            }

            sprintf(journals, "<journal_name>%s</journal_name>", journal->d_name);
        }

        closedir(user_directory);
    }

    if(strcmp(journals, "") == 0) {
        return get_command_result(OPERATION_FAIL, "No journals found.", NULL, 0);
    }

    return get_command_result(OPERATION_SUCCESS, "Journals retrieved succesfully.", journals, current_journals_size);
}


command_result* import_journal(user_id id, char* journal_name, char* journal_data, size_t journal_data_size) {
    command_result* result = prepare_journal_creation(id);
    if(result) {
        return result;
    }

    char journal_path[1024];
    get_journal_path(id, journal_name, journal_path);
    
    int journal_fd = open(journal_path, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP);
    if(journal_fd == OPERATION_FAIL) {
        log_error("The journal %s could not be created before import! Error: %s.", journal_path, strerror(errno));
        return get_command_result(OPERATION_FAIL, "Journal could not be imported.", (char*)NULL, 0);
    }

    if(write(journal_fd, journal_data, journal_data_size) == OPERATION_FAIL) {
        log_error("The journal %s could not be imported because content of it cannot be written! Error: %s.", journal_path, strerror(errno));
        return get_command_result(OPERATION_FAIL, "Journal could not be imported.", (char*)NULL, 0);
    }
    
    close(journal_fd);
    log_debug("Journal %s imported succesfully.\n", journal_name);
    return get_command_result(OPERATION_SUCCESS, "Journal imported succesfully.", (char*)NULL, 0);
}


void modify_journal_internal(struct zip_t* journal_zip, struct zip_t* new_content_zip, char* journal_path) {
    char* content = NULL;
    int total_entries = zip_entries_total(new_content_zip);

    for(int i = 0; i < total_entries; i++) {
        if(zip_entry_openbyindex(new_content_zip, i) == OPERATION_FAIL) {
            log_warning("Failed to open new content zip entry %d!", i);
            continue;
        }
        
        if(zip_entry_isdir(new_content_zip)) {
            log_warning("New content zip entry %d is a folder!", i);
            zip_entry_close(new_content_zip);
            continue;
        }

        const char* entry_name = zip_entry_name(new_content_zip);
        if(!entry_name) {
            log_warning("Failed to get new content entry %d name!", i);
            zip_entry_close(new_content_zip);
            continue;
        }

        if(zip_entry_open(journal_zip, entry_name) == OPERATION_FAIL) {
            log_warning("Failed to open journal entry %s!", journal_path);
            zip_entry_close(new_content_zip);
            continue;
        }

        char* entry_data = NULL;
        size_t entry_data_size = 0;
        if(zip_entry_read(new_content_zip, (void**)&entry_data, &entry_data_size) < 0) {
            log_warning("Failed to read new content entry %s!", journal_path);
            zip_entry_close(new_content_zip);
            zip_entry_close(journal_zip);
            continue;
        }

        if(zip_entry_write(journal_zip, (void*)entry_data, entry_data_size) < 0) {
            log_warning("Failed to write journal entry %s!", journal_path);
        }

        zip_entry_close(journal_zip);
        zip_entry_close(new_content_zip);
        free(entry_data);
    }
}

command_result* modify_journal(user_id id, char* journal_name, char* new_content, size_t new_content_size) {    
    char journal_path[1024];
    get_journal_path(id, journal_name, journal_path);

    struct zip_t* journal_zip = zip_open(journal_path, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    if(!journal_zip) {
        log_error("The journal %s could not be found! Error: %s.", journal_path, strerror(errno));
        return get_command_result(OPERATION_FAIL, "Journal could not be found on the server.", (char*)NULL, 0);
    }

    struct zip_t *new_content_zip = zip_stream_open(new_content, new_content_size, ZIP_DEFAULT_COMPRESSION_LEVEL, 'r');
    if(!new_content_zip) {
        log_error("Sent journal could not be opened! Error: %s.", journal_path, strerror(errno));
        return get_command_result(OPERATION_FAIL, "Sent journal could not be opened.", (char*)NULL, 0);
    }

    modify_journal_internal(journal_zip, new_content_zip, journal_path);
    zip_close(journal_zip);
    zip_close(new_content_zip);
    return get_command_result(OPERATION_SUCCESS, "Journal modified succesfully.", (char*)NULL, 0);
}


command_result* delete_journal(user_id id, char* journal_name) {
    char journal_path[1024];
    get_journal_path(id, journal_name, journal_path);

    if(remove(journal_path) == OPERATION_FAIL) {
        log_error("Journal %s could not be deleted! Error: %s.", journal_path, strerror(errno));
        return get_command_result(OPERATION_FAIL, "Journal could not be deleted.", (char*)NULL, 0);
    }

    log_info("Journal %s was deleted successfully!", journal_path);
    return get_command_result(OPERATION_SUCCESS, "Journal deleted succesfully.", (char*)NULL, 0);
}


operation_status disconnect_client(user_id id, int client_fd) {
    log_info("Client with id %lu and socket id %d has disconnected!\n", id, client_fd);
    return (operation_status)close(client_fd);
}