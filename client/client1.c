#define  _GNU_SOURCE

#include "server_messages.h"
#include "global.h"

#include <stdio.h>		
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>		
#include <arpa/inet.h>		
#include <sys/socket.h>     

static USER_ID user_id = -1;

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
	
	if(strcmp(command, "generate-id\n") == 0) {						
		generate_id(MESSAGE_HEADER* header, int socket_fd);
	}else if(strcmp(command, "create-journal\n") == 0) {						
		create_journal(MESSAGE_HEADER* header, int socket_fd);
	}else if(strcmp(command, "retrieve-journal\n") == 0) {						
		retrieve_journal();
	}else if(strcmp(command, "import-journal\n") == 0) {						
		import_journal();
	}else if(strcmp(command, "modify-journal\n") == 0) {						
		modify_journal();
	}else if(strcmp(command, "delete-journal\n") == 0) {						
		delete_journal(MESSAGE_HEADER* header, int socket_fd);
	}else if(strcmp(command, "disconnect\n") == 0) {						
		disconnect_client(MESSAGE_HEADER* header, int socket_fd);
	}else{
		printf("Invalid command!\n");
	}
}


OPERATION_STATUS generate_id(MESSAGE_HEADER* header, int socket_fd){
	MESSAGE* message = (MESSAGE*)malloc(sizeof(MESSAGE));
    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)malloc(sizeof(MESSAGE_HEADER));
    message_header->message_type = GENERATE_ID;
    message_header->user_id = user_id;

    message->header = message_header;

	send(socket_fd);

	char line[512];
	size_t received = recv(socket_fd, &line, 512, 0);
	line[received] = "\0";

	MESSAGE_HEADER* response_header = parse_header(line);
	if(!response_header) {
		printf("Invalid response\n");
	}

	user_id = header->user_id;
	printf("Generated id %lu\n", user_id);
	return OPERATION_SUCCESS;
}


OPERATION_STATUS create_journal(MESSAGE_HEADER* header, int socket_fd){
	MESSAGE* message = (MESSAGE*)malloc(sizeof(MESSAGE));
    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)malloc(sizeof(MESSAGE_HEADER));
    message_header->message_type = CREATE_JOURNAL;
    message_header->user_id = user_id;

    message->header = message_header;

	send(socket_fd);

	char line[512];
	size_t received = recv(socket_fd), &line, 512, 0;
	line[received] = "\0";

	MESSAGE* response_message = parse_message(line);
	if(!response_message) {
		printf("Invalid response\n");
	}

	printf("Created journal %s\n", message->content);
	return OPERATION_SUCCESS;
}


OPERATION_STATUS retrieve_journal(){


}


OPERATION_STATUS import_journal(){


}


OPERATION_STATUS modify_journal(){


}


OPERATION_STATUS delete_journal(MESSAGE_HEADER* header, int socket_fd){

	MESSAGE* message = (MESSAGE*)malloc(sizeof(MESSAGE));
    MESSAGE_HEADER* message_header = (MESSAGE_HEADER*)malloc(sizeof(MESSAGE_HEADER));
    message_header->message_type = DELETE_JOURNAL;
    message_header->user_id = user_id;

    message->header = message_header;

	send(socket_fd);

	char line[512];
	size_t received = recv(socket_fd), &line, 512, 0;
	line[received] = "\0";

	MESSAGE* response_message = parse_message(line);
	if(!response_message) {
		printf("Invalid response\n");
	}

	printf("Deleted journal %s\n", message->content);
	return OPERATION_SUCCESS;
}


OPERATION_STATUS disconnect_client(MESSAGE_HEADER* header, int socket_fd){
	send(socket_fd);
	close(socket_fd);

	printf("Client %lu has disconnected!\n", header->user_id);
    return (OPERATION_STATUS)close(sochet_fd);
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