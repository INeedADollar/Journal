#include "inet_thread.h"
#include "message.h"
#include "async_tasks.h"
#include "utils.h"
#include "commands.h"

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
#include <fcntl.h>

typedef struct {
	size_t buffered_message_size;
	char* buffered_message;
	message_header* header;
} client_data;

client_data clients_data[FD_SETSIZE];


operation_status init_server() {										
	int server_socket_fd;
	struct sockaddr_in server_addr;
	struct hostent* he;

	if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {				
		LOG_ERROR("The server socket cannot be open. Error: ", strerror(errno));
		return OPERATION_FAIL;
	}

    int reuse_address = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_address, 4) < 0) {
        LOG_ERROR("Cannot set reuse address option on socket. Error: ", strerror(errno));
        return OPERATION_FAIL;
    }

	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = htons(5000);

	if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {        
		LOG_ERROR("Could not bind the server socket! Error: ", strerror(errno));
		return OPERATION_FAIL;
	}
	
	fcntl(server_socket_fd, F_SETFL, O_NONBLOCK);
	return server_socket_fd;
}


operation_status accept_new_client(fd_set* active_set, int server_socket_fd) {					
	int client_addr_len, client_socket_fd;
	struct sockaddr_in client_addr;
	
	client_addr_len = sizeof(client_addr);
	bzero((char*)&client_addr, sizeof(client_addr));
	client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_len);		

	if (client_socket_fd < 0) {
		LOG_ERROR("Connection with client failed. Error: ", strerror(errno))
		return OPERATION_FAIL;
	}
	
	LOG_INFO("Client %d connected.\n", client_socket_fd)
    FD_SET(client_socket_fd, &active_set);
	return OPERATION_SUCCESS;
}


operation_status read_message_part(int socket_fd, char* buffer, size_t* current_size, size_t message_size) {
    char part[1024];
	size_t size_to_read_with_header = message_length - *current_size < 1024 ? message_length - *current_size : 1024;
    size_t size_to_read = message_size == 0 ? 100 : size_to_read_with_header;
	size_t received = recv(socket_fd, &part, size_to_read, 0);

	if(received == OPERATION_FAIL) {
		LOG_ERROR("Could not read from client %d", socket_fd)
		disconnect_client(socket_fd);
		return OPERATION_FAIL;
	}

    strcat(buffer, part);
	*current_size += received;
	return OPERATION_SUCCESS;
}

operation_status handle_client_command(fd_set* active_set, int client_socket_fd) {
	message_header* header = NULL;
	int received_len = 0;
	char read_message[1024];

	if(!clients_data[client_socket_fd].header) {
		read_message_part(client_socket_fd, read_message, &received_len, 0);
		read_message[received_len] = '\0';
		
		header = parse_header(read_message);
		if(header == NULL) {
			LOG_ERROR("Could not parse header from: %s, client_socket_id: %d", read_message, client_socket_fd);
			return OPERATION_FAIL;
		}

		if(header->type == RETRIEVE_JOURNAL || header->message_type == IMPORT_JOURNAL || header->message_type == MODIFY_JOURNAL) {
			clients_data[client_socket_fd].header = header;
			clients_data[client_socket_fd].buffered_message_size = received_len;
			clients_data[client_socket_fd].buffered_message = (char*)malloc(header->length);
			strcpy(clients_data[client_socket_fd].buffered_message, read_message);
		}
	}
    else if(clients_data[client_socket_fd].buffered_message_size == clients_data[client_socket_fd].header->length) {
		size_t current_size = clients_data[client_socket_fd].buffered_message_size;
		clients_data[client_socket_fd].buffered_message[current_size] = '\0';
		message* message = parse_message(client_socket_fd, clients_data[client_socket_fd].buffered_message);

		char message_copy[clients_data[client_socket_fd].buffered_message_size];
		strcpy(message_copy, clients_data[client_socket_fd].buffered_message);

		free(clients_data[client_socket_fd].header);
		free(clients_data[client_socket_fd].buffered_message);
		clients_data[client_socket_fd].buffered_message_size = 0;

		if(!message) {
			LOG_ERROR("Could not parse message from: %s, user_id: %lu, client_socket_id: %d", message_copy, header->user_id, client_socket_fd)
			return OPERATION_FAIL;
		}

		return enqueue_message(message);
	}

	read_message_part(client_socket_fd, read_message, &received_len, header->length);
	if(!clients_data[client_socket_fd].header) {
		read_message[received_len] = '\0';
		message* message = parse_message(client_socket_fd, read_message);
		if(!message) {
			LOG_ERROR("Could not parse message from: %s, user_id: %lu, client_socket_id: %d", read_message, header->user_id, client_socket_fd)
			return OPERATION_FAIL;
		}

		free(header);
		return check_message_and_run_command(message);
	}

	return OPERATION_SUCCESS;
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
		LOG_ERROR("Could not listen for clients. Error: ", strerror(errno));
		pthread_exit(NULL);
	}	

	LOG_INFO("Waiting for clients to connect...");	

	while(!STOP_SERVER) {
        read_sockets_set = active_sockets_set;
        if(select(FD_SETSIZE, &read_sockets_set, NULL, NULL, NULL) < 0) {
			LOG_ERROR("Select failed. Error: ", strerror(errno))
            pthread_exit(NULL);
        }

        for (int fd = 0; fd < FD_SETSIZE; fd++) {
            if(FD_ISSET(fd, &read_sockets_set)) {
                if(fd == server_socket_fd) {
					LOG_DEBUG("Entering accept_new_client");
                    accept_new_client(&active_sockets_set, server_socket_fd);
					LOG_DEBUG("Exiting accept_new_client");

                }
                else {
					LOG_DEBUG("Entering handle_client_command");
                    handle_client_command(&active_sockets_set, fd);
					LOG_DEBUG("Exiting handle_client_command");
                }
            }
		}

		for(int i = 0; i < 100 - tasks_running_count(); i++) {
			message* message = dequeue_message();
			if(!message) {
				break;
			}

			check_message_and_run_command(message);
		}				
	}
}