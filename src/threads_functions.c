#include "threads_functions.h"
#include "server_messages.h"

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

static int stop_threads = 0;

int init_server() {										
	int socket_fd;
	struct sockaddr_in server_addr;
	struct hostent* he;

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {				
		perror("The server socket cannot be open.");
		return -1;
	}

    int reuse_address = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_address, 4) < 0) {
        perror("Cannot set reuse address option on socket.");
        return -1;
    }

	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_addr.sin_port = htons(5000);

	if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {        
		perror("Could not bind the server socket!");
		return -1;
	}
	
	return socket_fd;
}

void accept_and_connect_new_client(fd_set* active_set, int server_socket_fd) {					
	int client_addr_len, client_socket_fd;
	struct sockaddr_in client_addr;
	
	client_addr_len = sizeof(client_addr);
	bzero((char*)&client_addr, sizeof(client_addr));
	client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_len);		

	if (client_socket_fd < 0) {
	  perror("Connection with client failed");
	  exit(-1);
	}
	
	printf("Client %d connected.\n", client_socket_fd);
    FD_SET(client_socket_fd, &active_fd_set);
}

void handle_socket_operation(fd_set* active_set, int client_socket_fd) {
    int received_len;
	char line[512];
	char response[512];
	
	received_len = recv(*client_socket_fd, &line, 512, 0);
}

void inet_thread() {
    fd_set active_sockets_set, read_sockets_set;
    int socket_fd = init_server();
	
	if(socket_fd < 0) {
		pthread_exit(NULL);
	}

    FD_ZERO(&active_sockets_set);
    FD_SET(socket_fd, &active_sockets_set);

	listen(socket_fd, 5);	
	puts("Waiting for clients to connect...");	

	while(!stop_threads) {
        read_sockets_set = active_sockets_set;
        if(select (FD_SETSIZE, &read_sockets_set, NULL, NULL, NULL) < 0) {
            pthread_exit (NULL);
        }

        for (int fd = 0; fd < FD_SETSIZE; fd++) {
            if(FD_ISSET(fd, &read_sockets_set)) {
                if(fd == socket_fd) {
                    accept_and_connect_new_client(&active_sockets_set, socket_fd);
                }
                else {
                    handle_connection_operation(&active_sockets_set, fd);
                }
            }
        }					
	}
}