#include "inet_thread.h"
#include "server_messages.h"
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

OPERATION_STATUS handle_socket_operation(fd_set* active_set, int client_socket_fd) {
    int received_len;
	char line[100];
	char* response;
	
	received_len = recv(*client_socket_fd, &line, 100, 0);

    MESSAGE_HEADER* header = parse_header(line);
    if(header == NULL) {
        return;
    }

	OPERATION_STATUS status = OPERATION_SUCCESS;
	switch (header->message_type) {
	case GENERATE_ID:
		generate_id(client_socket_fd);
		break;
	case CREATE_JOURNAL:
		create_journal(header, line, client_socket_fd);
		break;
    case RETRIEVE_JOURNAL:
		break;
    case IMPORT_JOURNAL:
		break;
    case MODIFY_JOURNAL:
		break;
    case DELETE_JOURNAL:
		delete_journal(client_socket_fd);
		break;
    case DISCONNECT_CLIENT:
		disconnect_client(header, client_socket_fd);
		break;
	default:
		break;
	}

	return status;
}

void inet_thread() {
    fd_set active_sockets_set, read_sockets_set;
    int server_socket_fd = init_server();
	
	if(server_socket_fd < 0) {
		pthread_exit(NULL);
	}

    FD_ZERO(&active_sockets_set);
    FD_SET(server_socket_fd, &active_sockets_set);

	listen(server_socket_fd, 5);	
	puts("Waiting for clients to connect...");	

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