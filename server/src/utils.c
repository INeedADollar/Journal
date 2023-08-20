#include "utils.h"
#include "logger.h"

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>


void get_command_type_string(command_types type, char* buffer) {
    switch(type) {
    case GENERATE_ID:
        strcpy(buffer, "GENERATE_ID");
        break;
    case CREATE_JOURNAL:
        strcpy(buffer, "CREATE_JOURNAL");
        break;
    case IMPORT_JOURNAL:
        strcpy(buffer, "IMPORT_JOURNAL");
        break;
    case RETRIEVE_JOURNAL:
        strcpy(buffer, "RETRIEVE_JOURNAL");
        break;
    case RETRIEVE_JOURNALS:
        strcpy(buffer, "RETRIEVE_JOURNALS");
        break;
    case MODIFY_JOURNAL:
        strcpy(buffer, "MODIFY_JOURNAL");
        break;
    case DELETE_JOURNAL:
        strcpy(buffer, "DELETE_JOURNAL");
        break;
    case DISCONNECT_CLIENT:
        strcpy(buffer, "DISCONNECT_CLIENT");
        break;
    case INVALID_COMMAND:
        strcpy(buffer, "INVALID_COMMAND");
        break;
    default:
        break;
    }
}


operation_status send_command_result_message(user_id id, command_result* result) { 
    size_t status_size = result->type == OPERATION_SUCCESS ? 18 : 15;
    size_t content_size = strlen(result->status_message) + result->additional_data_size + status_size + 190;
    char content[content_size + 8];
    char status[18];

    if(result->type == OPERATION_SUCCESS) {
        strcpy(status, "OPERATION_SUCCESS");
    }
    else {
        strcpy(status, "OPERATION_FAIL");
    }

    sprintf(content, "Content\nstatus=<journal_response_value>%s</journal_response_value>\nstatus-message=<journal_response_value>%s</journal_response_value>\n", status, result->status_message); 

    if(result->additional_data) {
        char* content_end = content + (content_size - result->additional_data_size - 67);
        sprintf(content_end, "additional-data=<journal_response_value>%s</journal_response_value>\n", result->additional_data);
    }
    else {
        content_size -= 67;
    }

    char header[110];
    char command_type[17];

    get_command_type_string(result->type, command_type);
    sprintf(header, "Header\ncommand-type<::::>%s\ncontent-length<::::>%zu\nuser-id<::::>%d\n", command_type, content_size, id);

    size_t message_size = strlen(header) + content_size + 8;
    char message[message_size];
    sprintf(message, "%s%s", header, content);

    ssize_t send_result = send(id / 1000, message, message_size, 0);
    if(send_result == OPERATION_FAIL) {
        log_error("Failed to send actual response to client with id %lu", id);
        return OPERATION_FAIL;
    }

    return OPERATION_SUCCESS;
}


message_content_node_data* extract_value_from_content(message_content* content, char* key) {
    if(!content) {
        return NULL;
    }

    message_content_node* node = content->head;
    while(node) {
        if(strcmp(node->key, key) == 0) {
            return node->node_data;
        }
    }

    return NULL;
}


// https://stackoverflow.com/questions/10347689/how-can-i-check-whether-a-string-ends-with-csv-in-c
int string_ends_with(const char * str, const char * suffix) {
  int str_len = strlen(str);
  int suffix_len = strlen(suffix);

  return 
    (str_len >= suffix_len) &&
    (0 == strcmp(str + (str_len-suffix_len), suffix));
}