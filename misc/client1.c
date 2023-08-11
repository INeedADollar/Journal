#define  _GNU_SOURCE

#include <stdio.h>		
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>		
#include <arpa/inet.h>		
#include <sys/socket.h>     

int init_client() {												
	int socket_fd;
	struct sockaddr_in server_addr;
	struct hostent* he;
	
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {	
		perror("The server socket cannot be open!");
		return -1;
	}

	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.2"); 
	server_addr.sin_port = htons(5000);

	if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {	 
		perror("Could not connect client socket to server!");
		return -1;
	}
	
	return socket_fd;
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

	puts("Client connected to server");

	while(1) {
		send_command_and_print_response(socket_fd);
	}
	
	return 1;
}