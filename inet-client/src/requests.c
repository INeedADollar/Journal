#define _GNU_SOURCE 

#include "requests.h"
#include "logger.h"

#include <unistd.h>
#include <arpa/inet.h>		
#include <sys/socket.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


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
    sprintf(message, "Header\ncommand-type<::::>%s\ncontent-length<::::>%d\nuser-id<::::>%lu\nContent\n",
        request_type_str, content_size, id);

    size_t current_message_size = strlen(message);
    memcpy((void*)(message + strlen(message), (void*)content, content_size);

    *message_size = current_message_size + content_size;
    return message;
}


void send_request(char* message, size_t message_size) {
    ssize_t send_result = send(socket_fd, message, message_size, 0);
    if(send_result == -1) {
        log_error("Send error. Connection with server was closed. Error: %s", strerror(errno));
        exit(-1);
    }
}


response* get_response() {
    char initial_message[110];
    ssize_t received = recv(socket_fd, &initial_message, 110, 0);

    if(received == -1) {
        log_error("Read error. Connection with server was closed. Error: %s", strerror(errno));
        exit(-1);
    }

    initial_message[received] = '\0';

    char request_type[18];
    size_t content_length;
    unsigned long user_id;
    int vars_count = sscanf(initial_message, "Header\ncommand-type<::::>%s\ncontent-length<::::>%d\nuser-id<::::>%lu\nContent\n",
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

    content_start += 9;

    size_t received_content_size = received - (size_t)(content_start - initial_message);
    char content[content_length];
    char part[1024];

    while(received_content_size < content_length) {
        size_t size_to_read = 1024;
        if(received_content_size + size_to_read > content_length) {
            size_to_read = content_length - received_content_size;
        }

        received = recv(socket_fd, &part, 1024, 0);
        if(received == -1) {
            log_error("%s read error. Connection with server was closed. Error: %s", request_type, strerror(errno));
            exit(-1);
        }

        memcpy((void*)(content + received_content_size), (void*)part, received);
        received_content_size += received;
    }

    content[content_length] = '\0';

    response* resp = (response*)malloc(sizeof(response));
    char* status = strstr(content, "status=<journal_response_value>");
    if(!status) {
        log_warning("Invalid content received: %s", content);
        free(resp);
        return NULL;
    }

    status += 32;
    char* status_end_tag = strstr(status, "</journal_response_value>\n");
    if(!status_end_tag) {
        log_warning("Invalid content received: %s", content);
        free(resp);
        return NULL;
    }

    char status_str[18];
    memcpy((void*)status_str, (void*)status, (size_t)(status_end_tag - status));

    if(strcmp(status_str, "OPERATION_SUCCESS") == 0) {
        resp->status = SUCCESS;
    }
    else {
        resp->status = FAIL;
    }

    char* status_message = strstr(status_end_tag + 27, "status-message=<journal_response_value>");
    if(!status_message) {
        log_warning("Invalid content received: %s", content);
        free(resp);
        return NULL;
    }

    status += 40;
    char* status_message_end_tag = strstr(status, "</journal_response_value>\n");
    if(!status_message_end_tag) {
        log_warning("Invalid content received: %s", content);
        free(resp);
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

    additional_data += 41;
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

    return resp;
}


void get_user_id() {
    size_t request_message_size;
    char* request_message = get_message(GENERATE_ID, (char*)NULL, 0, &request_message_size);
    send_request(request_message, request_message_size);

    response* resp = get_response();
    if(!resp) {
        exit(-1);
    }

    resp->data[resp->data_size] = '\0';
    id = strtoul(resp->data, (char**)NULL, 10);
    free(resp->status_message);
    free(resp->data);
    free(resp);
    
    if(user_id == 0 || errno == ERANGE) {
        log_error("Could not register client with the server.");
        disconnect_client();
        exit(-1);
    }
}


void init_requests(char* server_address, int port, notification_callback callback) {
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
    nottif_callback = callback;
    get_user_id();
}


response* create_journal(char* journal_name) {
    size_t journal_name_size = strlen(journal_name);
    char content[journal_name_size + 14];
    sprintf(content, "journal-name=%s", journal_name);

    size_t request_message_size;
    char* request_message = get_message(CREATE_JOURNAL, (char*)NULL, 0, &request_message_size);
    send_request(request_message, request_message_size);

    return get_response();
}


response* retrieve_journal(char* journal_name) {
    size_t journal_name_size = strlen(journal_name);
    char content[journal_name_size + 14];
    sprintf(content, "journal-name=%s", journal_name);

    size_t request_message_size;
    char* request_message = get_message(RETRIEVE_JOURNAL, (char*)NULL, 0, &request_message_size);
    send_request(request_message, request_message_size);

    return get_response();
}


response* retrieve_journals() {
    size_t request_message_size;
    char* request_message = get_message(RETRIEVE_JOURNAL, (char*)NULL, 0, &request_message_size);
    send_request(request_message, request_message_size);

    return get_response();
}

response* import_journal(char* journal_name, char* journal_content, size_t journal_content_size) {

}

response* modify_journal(char* journal_name, char* modified_pages, size_t modified_pages_size) {

}

response* delete_journal(char* journal_name) {
    size_t journal_name_size = strlen(journal_name);
    char content[journal_name_size + 14];
    sprintf(content, "journal-name=%s", journal_name);

    size_t request_message_size;
    char* request_message = get_message(DELETE_JOURNAL, content, 0, &request_message_size);
    send_request(request_message, request_message_size);

    return get_response();
}


void disconnect_client() {
    size_t request_message_size;
    char* request_message = get_message(DISCONNECT_CLIENT, (char*)NULL, 0, &request_message_size);
    send_request(request_message, request_message_size);

    close(socket_fd);
}