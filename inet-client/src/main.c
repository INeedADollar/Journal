#define  _GNU_SOURCE

#include "logger.h"

#include <stdio.h>		
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>		
#include <arpa/inet.h>		
#include <sys/socket.h> 
#include <string.h>
#include <errno.h>


int STOP_CLIENT = 0;

void stop_client(int signal) {
    log_info("Stopping client...");
    STOP_CLIENT = 1;
}


int init_client() {												
	int socket_fd;
	struct sockaddr_in server_addr;
	struct hostent* he;
	
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {	
		log_error("The server socket cannot be open! Error: %s", strerror(errno));
		return -1;
	}

	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.2"); 
	server_addr.sin_port = htons(5000);

	if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {	 
		log_error("Could not connect client socket to server! Error: %s", strerror(errno));
		return -1;
	}
	
	return socket_fd;
}


void fetch_journals(int socket_fd) {
    char message
}


void send_command_and_print_response(int socket_fd) {				
	char* command = NULL;
	size_t len;
	
	printf(">>> ");
	getline(&command, &len, stdin);

	send(socket_fd, command, len, 0);
	
	if(strcmp(command, "disconnect\n") == 0) {						
		close(socket_fd);
		exit(0);
	}

	char received_line[512];
	int received_len;
	
	if(received_len = recv(socket_fd, &received_line, 512, 0)) {	
		received_line[received_len] = '\0';
		puts(received_line);
	}
}

int main(int argc, char * argv[]) {
	int socket_fd = init_client();
	
	if(socket_fd < 0) {
		return -1;
	}

	log_info("Client connected to server.");

    signal(SIGINT, stop_client);
	while(!STOP_CLIENT) {
		send_command_and_print_response(socket_fd);
	}
	
	return 1;
}