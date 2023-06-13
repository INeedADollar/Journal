#include <stdio.h>		
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>		
#include <arpa/inet.h>		
#include <sys/socket.h>        
#include <pthread.h>

int init_server() {										
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

	if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {        
		perror("Could not bind wanted address to server socket!");
		return -1;
	}
	
	return socket_fd;
}

void server_time(char* time_string) { 
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(time_string, "::: %d-%02d-%02d %02d:%02d:%02d :::", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void user_name(char* user_name_string) {						
	char* username = getlogin();
	strcpy(user_name_string, "::: ");
	strcat(user_name_string, username);
	strcat(user_name_string, " :::");
}

void echo_line(char* string_to_echo, char* response) {					
	int response_len = strlen(string_to_echo) + 4;
	strcpy(response, "::: ");
	strcat(response, string_to_echo);
	strcpy(response + response_len - 1, response + response_len);
	strcat(response, " :::");
}

void virus(char* virus_string) {							
	strcpy(virus_string, "You just infected your computer :x:");
}

void client_thread(int* socket_fd) {							
	int received_len;
	char line[512];
	char response[512];
	
	while(received_len = recv(*socket_fd, &line, 512, 0)) {
		line[received_len] = '\0';
		printf("::: Received: %s from client %d :::\n", line, *socket_fd);
		
		if(strcmp(line, "time\n") == 0) {						
			server_time((char*)&response);
		}
		else if(strcmp(line, "user\n") == 0) {						
			user_name((char*)&response);
		}
		else if(strcmp(line, "disconnect\n") == 0) {						
			break;
		}
		else if(strcmp(line, "virus\n") == 0) {						
			virus((char*)&response);
		}
		else {
			echo_line((char *)&line, (char*)&response);				
		}
		
		send(*socket_fd, response, strlen((char*)response), 0);
		printf("::: Sent %s to client %d :::\n", response, *socket_fd);
	}
	
	close(*socket_fd);									
	printf("Client %d disconnected\n", *socket_fd);
}

void accept_and_connect_new_client(int server_socket_fd) {					
	int client_addr_len, client_socket_fd;
	struct sockaddr_in client_addr;
	
	pthread_t thread_id;
	pthread_attr_t thread_attr;
	
	client_addr_len = sizeof(client_addr);
	bzero((char*)&client_addr, sizeof(client_addr));
	client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_len);		

	if (client_socket_fd < 0) {
	  perror("Connection with client failed");
	  exit(-1);
	}
	
	printf("Client %d connected.\n", client_socket_fd);
	
	int res = pthread_attr_init(&thread_attr);								
	if(res != 0) {
		perror("Could not create client thread attribute");
		close(client_socket_fd);
		return;
	}
	
	res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);				
	if(res != 0) {
		perror("Could not set detached attribute");
		close(client_socket_fd);
		return;
	}
	
	int* server_socket_fd_p = (int*)malloc(sizeof(int));
	*server_socket_fd_p = client_socket_fd;									
	
	res = pthread_create(&thread_id, (pthread_attr_t*)&thread_attr, (void * (*)(void *))client_thread, server_socket_fd_p);		
	if(res != 0) {
		perror("Could not create client thread");
		close(client_socket_fd);
		return;
	}
}

int main(int argc, char * argv[]) {
	int socket_fd = init_server();
	
	if(socket_fd < 0) {
		return -1;
	}

	puts("Waiting for clients to connect...");

	listen(socket_fd, 5);											
	while(1) {
		accept_and_connect_new_client(socket_fd);							
	}
	
	return 0;
}

