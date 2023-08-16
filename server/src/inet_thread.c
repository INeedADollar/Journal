#include "inet_thread.h"
#include "message.h"
#include "async_tasks.h"
#include "utils.h"
#include "commands.h"
#include "messages_queue.h"
#include "logger.h"

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
#include <errno.h>


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
		log_error("The server socket cannot be open. Error: %s.", strerror(errno));
		return OPERATION_FAIL;
	}

    int reuse_address = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_address, 4) < 0) {
        log_error("Cannot set reuse address option on socket. Error: %s.", strerror(errno));
        return OPERATION_FAIL;
    }

	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = htons(5000);

	if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {        
		log_error("Could not bind the server socket! Error: %s.", strerror(errno));
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
		log_error("Connection with client failed. Error: %s.", strerror(errno));
		return OPERATION_FAIL;
	}
	
	log_info("Client %d connected.\n", client_socket_fd);
    FD_SET(client_socket_fd, active_set);
	return OPERATION_SUCCESS;
}


operation_status read_message_part(int socket_fd, char* buffer, size_t* current_size, size_t message_size) {
    char part[1024];
	size_t size_to_read_with_header = message_size - *current_size < 1024 ? message_size - *current_size : 1024;
    size_t size_to_read = message_size == 0 ? 100 : size_to_read_with_header;
	size_t received = recv(socket_fd, &part, size_to_read, 0);

	if(received == OPERATION_FAIL) {
		log_error("Could not read from client %d.", socket_fd);
		disconnect_client(0, socket_fd);
		return OPERATION_FAIL;
	}

    strcat(buffer, part);
	*current_size += received;
	return OPERATION_SUCCESS;
}

operation_status handle_client_command(fd_set* active_set, int client_socket_fd) {
	message_header* header = NULL;
	size_t received_len = 0;
	char read_message[1024];

	if(!clients_data[client_socket_fd].header) {
		read_message_part(client_socket_fd, read_message, &received_len, 0);
		read_message[received_len] = '\0';
		
		header = parse_header(read_message);
		if(header == NULL) {
			log_error("Could not parse header from: %s, client_socket_id: %d.", read_message, client_socket_fd);
			return OPERATION_FAIL;
		}

		if(header->type == RETRIEVE_JOURNAL || header->type == IMPORT_JOURNAL || header->type == MODIFY_JOURNAL) {
			clients_data[client_socket_fd].header = header;
			clients_data[client_socket_fd].buffered_message_size = received_len;
			clients_data[client_socket_fd].buffered_message = (char*)malloc(header->length);
			strcpy(clients_data[client_socket_fd].buffered_message, read_message);
		}
	}
    else if(clients_data[client_socket_fd].buffered_message_size == clients_data[client_socket_fd].header->length) {
		size_t current_size = clients_data[client_socket_fd].buffered_message_size;
		clients_data[client_socket_fd].buffered_message[current_size] = '\0';
		message_t* message = parse_message(client_socket_fd, clients_data[client_socket_fd].buffered_message);

		char message_copy[clients_data[client_socket_fd].buffered_message_size];
		strcpy(message_copy, clients_data[client_socket_fd].buffered_message);

		free(clients_data[client_socket_fd].header);
		clients_data[client_socket_fd].header = NULL;
		
		free(clients_data[client_socket_fd].buffered_message);
		clients_data[client_socket_fd].buffered_message_size = 0;

		if(!message) {
			log_error("Could not parse message from: %s, user_id: %lu, client_socket_id: %d.", message_copy, header->client_id, client_socket_fd);
			return OPERATION_FAIL;
		}

		return enqueue_message(message);
	}

	read_message_part(client_socket_fd, read_message, &received_len, header->length);
	if(!clients_data[client_socket_fd].header) {
		read_message[received_len] = '\0';
		message_t* message = parse_message(client_socket_fd, read_message);
		if(!message) {
			log_error("Could not parse message from: %s, user_id: %lu, client_socket_id: %d.", read_message, header->client_id, client_socket_fd);
			return OPERATION_FAIL;
		}

		user_id id = header->client_id;
		free(header);
		command_result* result = check_message_and_run_command(message);
		if(!result) {
			return OPERATION_SUCCESS;
		}

		return send_command_result_message(id, result);
	}

	return OPERATION_SUCCESS;
}


void inet_thread(void* args) {
    fd_set active_sockets_set, read_sockets_set;
    int server_socket_fd = init_server();
	
	if(server_socket_fd < 0) {
		pthread_exit(NULL);
	}

    FD_ZERO(&active_sockets_set);
    FD_SET(server_socket_fd, &active_sockets_set);

	if(listen(server_socket_fd, 5) < 0) {
		log_error("Could not listen for clients. Error: %s.", strerror(errno));
		pthread_exit(NULL);
	}	

	log_info("Waiting for clients to connect...");	

	while(!STOP_SERVER) {
        read_sockets_set = active_sockets_set;
        if(select(FD_SETSIZE, &read_sockets_set, NULL, NULL, NULL) < 0) {
			log_error("Select failed. Error: %s.", strerror(errno));
            pthread_exit(NULL);
        }

        for (int fd = 0; fd < FD_SETSIZE; fd++) {
            if(FD_ISSET(fd, &read_sockets_set)) {
                if(fd == server_socket_fd) {
					log_debug("Entering accept_new_client...");
                    accept_new_client(&active_sockets_set, server_socket_fd);
					log_debug("Exiting accept_new_client...");

                }
                else {
					log_debug("Entering handle_client_command...");
                    handle_client_command(&active_sockets_set, fd);
					log_debug("Exiting handle_client_command...");
                }
            }
		}

		for(int i = 0; i < 100 - tasks_running_count(); i++) {
			message_t* message = dequeue_message();
			if(!message) {
				break;
			}

			check_message_and_run_command(message);
		}				
	}
}