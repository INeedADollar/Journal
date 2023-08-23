#ifndef _GNU_SOURCE
#define _GNU_SOURCE 
#endif // _GNU_SOURCE

#include "requests.h"
#include "logger.h"

#include <unistd.h>
#include <arpa/inet.h>		
#include <sys/socket.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>


typedef struct {
    request_type type;
    char* journal_name;
    char* content_key;
    char* data;
    size_t data_size;
} async_notif_thread_args;

static notification_callback nottif_callback;
static int socket_fd;
static unsigned long id = -1;


void get_request_type_string(request_type type, char* buffer) {
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


request_type get_request_type(char* type_str) {
    if(strcmp(type_str, "GENERATE_ID") == 0) {
        return GENERATE_ID;
    }

    if(strcmp(type_str, "CREATE_JOURNAL") == 0) {
        return CREATE_JOURNAL;
    }

    if(strcmp(type_str, "RETRIEVE_JOURNAL") == 0) {
        return RETRIEVE_JOURNAL;
    }
    
    if(strcmp(type_str, "RETRIEVE_JOURNALS") == 0) {
        return RETRIEVE_JOURNALS;
    }

    if(strcmp(type_str, "IMPORT_JOURNAL") == 0) {
        return IMPORT_JOURNAL;
    }

    if(strcmp(type_str, "MODIFY_JOURNAL") == 0) {
        return MODIFY_JOURNAL;
    }

    if(strcmp(type_str, "DELETE_JOURNAL") == 0) {
        return DELETE_JOURNAL;
    }

    if(strcmp(type_str, "DISCONNECT_CLIENT") == 0) {
        return DISCONNECT_CLIENT;
    }

    return INVALID_COMMAND;
}


char* get_request_message(request_type type, char* content, size_t content_size, size_t* message_size) {
    char* message = (char*)malloc(110 + content_size);
    char request_type_str[18];

    get_request_type_string(type, request_type_str);
    sprintf(message, "Header\ncommand-type<::::>%s\ncontent-length<::::>%zu\nuser-id<::::>%lu\nContent\n",
        request_type_str, content_size, id);

    size_t current_message_size = strlen(message);
    if(content) {
        memcpy((void*)(message + current_message_size), (void*)content, content_size);
    }

    *message_size = current_message_size + content_size;
    return message;
}


void send_request(char* message, size_t message_size) {
    log_info(message);
    ssize_t send_result = send(socket_fd, message, message_size, 0);
    if(send_result == -1) {
        log_error("Send error. Connection with server was closed. Error: %s", strerror(errno));
        exit(-1);
    }
}


response* get_response() {
    char initial_message[110];
    ssize_t received = recv(socket_fd, &initial_message, 110, 0);

    if(received < 1) {
        log_error("Read error. Connection with server was closed. Error: %s", strerror(errno));
        exit(-1);
    }

    initial_message[received] = '\0';

    char request_type[18];
    size_t content_length;
    unsigned long user_id;
    int vars_count = sscanf(initial_message, "Header\ncommand-type<::::>%s\ncontent-length<::::>%zu\nuser-id<::::>%lu\nContent\n",
        request_type, &content_length, &user_id);

    if(vars_count < 3 || vars_count == EOF) {
        log_warning("Header could not be parsed from %s.", initial_message);
        return NULL;
    }

    char* content_start = strstr(initial_message, "Content\n");
    if(!content_start) {
        log_warning("No content found in message %s", initial_message);
        return NULL;
    }

    content_start += 8;
    *(content_start - 1) = '\0';

    log_info(initial_message);
    size_t received_content_size = strlen(content_start);
    char content[content_length];
    char part[1024];

    log_info("%zu", received_content_size);
    if(received_content_size > 0) {
        strcpy(content, (char*)(content_start));
    }

    log_info(content);
    while(received_content_size < content_length) {
        size_t size_to_read = 1024;
        if(received_content_size + size_to_read > content_length) {
            size_to_read = content_length - received_content_size;
        }

        received = recv(socket_fd, &part, size_to_read, 0);

        if(received < 1) {
            log_error("%s read error. Connection with server was closed. Error: %s", request_type, strerror(errno));
            exit(-1);
        }

        memcpy((void*)(content + received_content_size), (void*)part, received);
        received_content_size += received;

        log_info("%zu %zu", received_content_size, content_length);
    }

    content[content_length] = '\0';

    response* resp = (response*)calloc(1, sizeof(response));
    char* status = strstr(content, "status=<journal_response_value>");
    if(!status) {
        log_warning("Invalid content received: %s", content);
        delete_response(resp);
        return NULL;
    }

    status += 31;
    char* status_end_tag = strstr(status, "</journal_response_value>\n");
    if(!status_end_tag) {
        log_warning("Invalid content received: %s", content);
        delete_response(resp);
        return NULL;
    }

    log_info("%18s", status);
    if(strncmp(status, "OPERATION_SUCCESS", 17) == 0) {
        resp->status = SUCCESS;
    }
    else {
        resp->status = FAIL;
    }

    char* status_message = strstr(status_end_tag + 26, "status-message=<journal_response_value>");
    if(!status_message) {
        log_warning("Invalid content received: %s", content);
        delete_response(resp);
        return NULL;
    }

    status_message += 39;
    char* status_message_end_tag = strstr(status_message, "</journal_response_value>\n");
    if(!status_message_end_tag) {
        log_warning("Invalid content received: %s", content);
        delete_response(resp);
        return NULL;
    }

    size_t status_message_size = (size_t)(status_message_end_tag - status_message);
    char* status_message_str = (char*)malloc(status_message_size + 1);
    memcpy((void*)status_message_str, (void*)status_message, status_message_size);

    resp->status_message = status_message_str;

    char* additional_data = strstr(status_message_end_tag, "additional-data=<journal_response_value>");
    if(!additional_data) {
        resp->data = NULL;
        resp->data_size = 0;

        return resp;
    }

    additional_data += 40;
    char* additional_data_end_tag = memmem((void*)additional_data, content_length - (size_t)(content - additional_data),
        (void*)"</journal_response_value>\n", 27);

    if(!additional_data_end_tag) {
        resp->data = NULL;
        resp->data_size = 0;

        return resp;
    }

    resp->data_size = (size_t)(additional_data_end_tag - additional_data);
    resp->data = (char*)malloc(resp->data_size);
    memcpy((void*)resp->data, (void*)additional_data, resp->data_size);
    resp->data[resp->data_size] = '\0';

    return resp;
}


void get_user_id() {
    FILE* id_file = fopen("./id.user", "r");
    if(id_file) {
        char id_str[15];
        fgets(id_str, 15, id_file);
        id = strtoul(id_str, (char**)NULL, 10);
    }
    else {
        size_t request_message_size;
        char* request_message = get_request_message(GENERATE_ID, (char*)NULL, 0, &request_message_size);
        send_request(request_message, request_message_size);

        response* resp = get_response();

        if(!resp) {
            log_error("Could not register client with the server. Invalid response received.");
            exit(-1);
        }

        resp->data[resp->data_size] = '\0';
        id = strtoul(resp->data, (char**)NULL, 10);
        delete_response(resp);
    }
    
    if(id == 0 || errno == ERANGE) {
        log_error("Could not register client with the server.");
        disconnect_client();
        exit(-1);
    }

    if(!id_file) {
        id_file = fopen("./id.user", "w+");
        if(!id_file) {
            log_warning("Could not save user id.");
            return;
        }

        fprintf(id_file, "%lu", id);
        fclose(id_file);
    }
    else {
        fclose(id_file);
    }
}


void init_requests(char* server_address, int port) {
    int new_socket_fd;
	struct sockaddr_in server_addr;
	struct hostent* he;
	
	if ((new_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {	
		log_error("The server socket cannot be open! Error: %s", strerror(errno));
		exit(-1);
	}

	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_address); 
	server_addr.sin_port = htons(port);

	if (connect(new_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {	 
		log_error("Could not connect client socket to server! Error: %s", strerror(errno));
		exit(-1);
	}
	
    socket_fd = new_socket_fd;
    get_user_id();
}


void register_notifications_callback(notification_callback callback) {
    nottif_callback = callback;
}


response* create_journal(char* journal_name) {
    size_t journal_name_size = strlen(journal_name);
    char content[journal_name_size + 62];
    sprintf(content, "journal-name=<journal_request_value>%s</journal_request_value>\n", journal_name);

    size_t request_message_size;
    char* request_message = get_request_message(CREATE_JOURNAL, content, journal_name_size + 62, &request_message_size);
    send_request(request_message, request_message_size);

    return get_response();
}


response* retrieve_journal(char* journal_name) {
    size_t journal_name_size = strlen(journal_name);
    char content[journal_name_size + 14];
    sprintf(content, "journal-name=<journal_request_value>%s</journal_request_value>\n", journal_name);

    size_t request_message_size;
    char* request_message = get_request_message(RETRIEVE_JOURNAL, content, 0, &request_message_size);
    send_request(request_message, request_message_size);

    return get_response();
}


response* retrieve_journals() {
    size_t request_message_size;
    char* request_message = get_request_message(RETRIEVE_JOURNALS, (char*)NULL, 0, &request_message_size);
    send_request(request_message, request_message_size);

    return get_response();
}


response* delete_journal(char* journal_name) {
    size_t journal_name_size = strlen(journal_name);
    char content[journal_name_size + 62];
    sprintf(content, "journal-name=<journal_request_value>%s</journal_request_value>\n", journal_name);

    size_t request_message_size;
    char* request_message = get_request_message(DELETE_JOURNAL, content, journal_name_size + 62, &request_message_size);
    send_request(request_message, request_message_size);

    return get_response();
}


void async_operation_thread(async_notif_thread_args* args) {
    size_t journal_name_size = strlen(args->journal_name);
    size_t content_key_size = strlen(args->content_key);
    size_t content_size = journal_name_size + args->data_size + content_key_size + 111;
    char content[content_size];

    if(args->data) {
        sprintf(content, "journal-name=<journal_request_value>%s</journal_request_value>\n%s=<journal_request_value>", args->journal_name, args->content_key);
        memcpy((void*)(content + journal_name_size + content_key_size + 85), (void*)args->data, args->data_size);
        memcpy((void*)(content + journal_name_size + content_key_size + args->data_size + 83), "</journal_request_value>\n", 25);
    }
    else {
        sprintf(content, "journal-name=<journal_request_value>%s</journal_request_value>\n", args->journal_name);
        content_size -= content_key_size + args->data_size + 59;
    }

    log_debug("%zu %zu", content_size, args->data_size);
    size_t request_message_size;
    char* request_message = get_request_message(args->type, content, content_size, &request_message_size);
    send_request(request_message, request_message_size);

    response* response = get_response();
    if(response) {
        nottif_callback(response, args->type);
    }

    free(args->journal_name);
    if(args->data) {
        free(args->content_key);
        free(args->data);
    }
    free(args);
}


int import_journal(char* journal_name, char* journal_content, size_t journal_content_size) {
    async_notif_thread_args* args = (async_notif_thread_args*)malloc(sizeof(async_notif_thread_args));
    args->journal_name = (char*)malloc(strlen(journal_name));
    args->content_key = (char*)malloc(16);
    args->data = (char*)malloc(journal_content_size);
    args->data_size = journal_content_size;
    args->type = IMPORT_JOURNAL;

    log_debug("Before strcpy");
    strcpy(args->journal_name, journal_name);
    strcpy(args->content_key, "journal-data");
    
    memcpy(args->data, journal_content, journal_content_size);
    log_debug("After strcpy");

    pthread_t thread;
    int res = pthread_create(&thread, (pthread_attr_t*)NULL, (void * (*)(void *))async_operation_thread, (void*)args);
    if(res != 0) {
        log_error("Could not create import journal thread. Error: %s", strerror(errno));
        return -1;
    }

    return 0;
}

int modify_journal(char* journal_name, char* modified_pages, size_t modified_pages_size) {
    async_notif_thread_args* args = (async_notif_thread_args*)malloc(sizeof(async_notif_thread_args));
    args->journal_name = (char*)malloc(strlen(journal_name));
    args->content_key = (char*)malloc(16);
    args->data = (char*)malloc(modified_pages_size);
    args->data_size = modified_pages_size;
    args->type = MODIFY_JOURNAL;

    strcpy(args->journal_name, journal_name);
    strcpy(args->content_key, "journal-content");
    strcpy(args->data, modified_pages);

    pthread_t thread;
    int res = pthread_create(&thread, (pthread_attr_t*)NULL, (void * (*)(void *))async_operation_thread, (void*)args);
    if(res != 0) {
        log_error("Could not create modify journal thread. Error: %s", strerror(errno));
        return -1;
    }

    return 0;
}


void disconnect_client() {
    size_t request_message_size;
    char* request_message = get_request_message(DISCONNECT_CLIENT, (char*)NULL, 0, &request_message_size);
    send_request(request_message, request_message_size);

    close(socket_fd);
}


void delete_response(response* resp) {
    if(resp->status_message) {
        free(resp->status_message);
    }

    if(resp->data) {
        free(resp->data);
    }

    free(resp);
}
