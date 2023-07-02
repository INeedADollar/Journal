#include "inet_thread.h"
#include "server_messages.h"
#include "async_tasks.h"
#include "read_tasks.h"
#include "global.h"
#include "operations.h"

#include <stdio.h>		
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>		
#include <arpa/inet.h>		
#include <sys/socket.h>   
#include <pthread.h>
#include <sys/select.h>
#include <signal.h>

OPERATION_STATUS init_server() {										
	int server_socket_fd;
	struct sockaddr_in server_addr;
	struct hostent* he;

	if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {				
		perror("The server socket cannot be open.");
		return OPERATION_FAIL;
	}

    int reuse_address = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_address, 4) < 0) {
        perror("Cannot set reuse address option on socket.");
        return OPERATION_FAIL;
    }

	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = htons(5000);

	if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {        
		perror("Could not bind the server socket!");
		return OPERATION_SUCCESS;
	}
	
	return server_socket_fd;
}

OPERATION_STATUS accept_new_client(fd_set* active_set, int server_socket_fd) {					
	int client_addr_len, client_socket_fd;
	struct sockaddr_in client_addr;
	
	client_addr_len = sizeof(client_addr);
	bzero((char*)&client_addr, sizeof(client_addr));
	client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_len);		

	if (client_socket_fd < 0) {
	  perror("Connection with client failed");
	  return OPERATION_FAIL;
	}
	
	printf("Client %d connected.\n", client_socket_fd);
    FD_SET(client_socket_fd, &active_fd_set);
	return OPERATION_SUCCESS;
}

void create_read_task(MESSAGE_HEADER* header, char* line, int client_socket_id) {
	READ_BIG_MESSAGE_TASK_ARGS* args = (READ_BIG_MESSAGE_TASK_ARGS*)malloc(sizeof(READ_BIG_MESSAGE_TASK_ARGS));
	args->header = header;
	args->initial_read_message = line;
	args->client_fd = client_socket_fd;

	return create_read_big_message_task(args);
}

OPERATION_STATUS handle_socket_operation(fd_set* active_set, int client_socket_fd) {
    int received_len;
	char* line = (char*)malloc(100);
	
	received_len = recv(*client_socket_fd, &line, 100, 0);
	line[received_len] = '\0';
	
    MESSAGE_HEADER* header = parse_header(line);
    if(header == NULL) {
		printf("Could not parse header from: %s, client_id: %d", line, client_socket_fd);
        return OPERATION_FAIL;
    }

	OPERATION_STATUS status = OPERATION_SUCCESS;
	switch (header->message_type) {
	case GENERATE_ID:
		generate_id(header, client_socket_fd);
		break;
	case CREATE_JOURNAL:
		create_journal(header, line, client_socket_fd);
		break;
    case RETRIEVE_JOURNAL:
    case IMPORT_JOURNAL:
    case MODIFY_JOURNAL:
		create_read_task(header, line, client_socket_fd);
		break;
    case DELETE_JOURNAL:
		delete_journal(header, client_socket_fd);
		break;
    case DISCONNECT_CLIENT:
		disconnect_client(header, client_socket_fd);
		break;
	default:
		break;
	}

	free(line);
	return status;
}

void check_queue_thread(void* args) {
	while(!STOP_SERVER) {
		if(tasks_running_count() < 100) {
			MESSAGE* message = dequeue_message();
			if(message) {
				create_async_task(message);
			}
		}
	}
}

void inet_thread() {
    fd_set active_sockets_set, read_sockets_set;
    int server_socket_fd = init_server();
	
	if(server_socket_fd < 0) {
		pthread_exit(NULL);
	}

    FD_ZERO(&active_sockets_set);
    FD_SET(server_socket_fd, &active_sockets_set);

	if(listen(server_socket_fd, 5) < 0) {
		pthread_exit(NULL);
	}	

	puts("Waiting for clients to connect...");	

	pthread_t thread_id;
	if(pthread_create(&thread_id, (pthread_attr_t*)NULL, (void * (*)(void *))check_queue_thread, (void*)NULL) != 0) {
		pthread_exit(NULL);
	}	

	while(!STOP_SERVER) {
        read_sockets_set = active_sockets_set;
        if(select(FD_SETSIZE, &read_sockets_set, NULL, NULL, NULL) < 0) {
            pthread_exit(NULL);
        }

        for (int fd = 0; fd < FD_SETSIZE; fd++) {
            if(FD_ISSET(fd, &read_sockets_set)) {
                if(fd == server_socket_fd) {
                    accept_new_client(&active_sockets_set, server_socket_fd);
                }
                else {
                    handle_connection_operation(&active_sockets_set, fd);
                }
            }
        }					
	}
}