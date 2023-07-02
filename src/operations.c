#include "operations.h"
#include "server_messages.h"
#include "global.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zip/src/zip.h>


void generate_id(int socket_fd){
    char response[512];
    char id_line[100];
    USER_ID new_id = time(NULL) + (random() % 1000); 
    
    sprintf(id_line, "new-id=%lu", new_id);
    sprintf(response, "Header\nmessage-type<::::>%d\nmessage-length<::::>%d\nContent\n", GENERATE_ID, strlen(id_line));
    strcat(response, id_line);

    send(socket_fd, response, strlen((char*)response), 0);
	printf("::: Created id=%lu for client %d :::\n", new_id, socket_fd);
}


void create_journal(MESSAGE_HEADER message_header, char* message_part, int socket_fd){
    int check;
    char dirname[512];
    char read_message[1024];
    char line[512];
    USER_ID user_id;

    recv(socket_fd, &line, 512, 0);
    MESSAGE* message = parse_message(socket_fd, read_message);

    if(!message){
        char content[512]; 
        sprintf(content, "status=%d\nmessage=%s\n", OPERATION_FAIL, "Recieved message is invalid!");
        message->header->message_length = strlen(content);
        message->header->message_type = CREATE_JOURNAL;
        message->content = content;

        send_message(message);
        return;
    }

    sprintf(dirname, "%lu", user_id);
    check = mkdir(dirname, 0777);
 
    if(!check)
        printf("::: Created directory %s for client with user-id %lu :::\n", dirname, user_id);
    else{
        perror("::: Unable to create directory %s for client with user-id %lu :::\n", dirname, user_id);

        char content[512]; 
        sprintf(content, "status=%d\nmessage=%s\n", OPERATION_FAIL, "Unable to create the journal!");
        message->header->message_length = strlen(content);
        message->header->message_type = CREATE_JOURNAL;
        message->content = content;

        send_message(message);
        return;
    }

    char* journal_name = strstr(message->content, "=");
    if(!journal_name){
        char content[512]; 
        sprintf(content, "status=%d\nmessage=%s\n", OPERATION_FAIL, "Journal name is invalid or missing!");
        message->header->message_length = strlen(content);
        message->header->message_type = CREATE_JOURNAL;
        message->content = content;

        send_message(message);
        return;
    }

    strcat(dirname, "/");
    strcat(dirname, journal_name);

    struct zip_t* zip = zip_open(dirname, ZIP_DEFAULT_COMPRESSION_LEVEL, "w+");
    zip_close(zip);

    char content[512]; 
    sprintf(content, "status=%d\nmessage=%s\n", OPERATION_SUCCESS, "Journal created successfully!");
    message->header->message_length = strlen(content);
    message->header->message_type = CREATE_JOURNAL;
    message->content = content;

    send_message(message);
}


void delete_journal(int socket_fd){
    char dirname[512];
    char read_message[1024];
    char line[512];
    USER_ID user_id;

    recv(socket_fd, &line, 512, 0);
    MESSAGE* message = parse_message(socket_fd, read_message);

    if(!message){
        char content[512]; 
        sprintf(content, "status=%d\nmessage=%s\n", OPERATION_FAIL, "Recieved message is invalid!");
        message->header->message_length = strlen(content);
        message->header->message_type = CREATE_JOURNAL;
        message->content = content;

        send_message(message);
        return;
    }

    char* journal_name = strstr(message->content, "=");
    if(!journal_name){
        char content[512]; 
        sprintf(content, "status=%d\nmessage=%s\n", OPERATION_FAIL, "Journal name is invalid or missing!");
        message->header->message_length = strlen(content);
        message->header->message_type = CREATE_JOURNAL;
        message->content = content;

        send_message(message);
        return;
    }

    if(remove(journal_name) == 0) {
        printf("The journal was deleted successfully!\n");
    }else{
        printf("The journal could not be deleted!\n");
    }

    char content[512]; 
    sprintf(content, "status=%d\nmessage=%s\n", OPERATION_SUCCESS, "Journal deleted successfully!");
    message->header->message_length = strlen(content);
    message->header->message_type = DELETE_JOURNAL;
    message->content = content;

    send_message(message);
}


void disconnect_client(MESSAGE_HEADER* header, int sochet_fd){
    close(sochet_fd);
    printf("Client %lu has disconnected!\n", header->user_id);
}