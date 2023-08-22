#ifndef _GNU_SOURCE
#define _GNU_SOURCE 
#endif // _GNU_SOURCE

#include <interface.h>
#include <requests.h>
#include <logger.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>


static char** journals = NULL;
static size_t journals_count = 0;

static char* journal_imported = NULL;
static char* journal_saved = NULL;

void notif_callback(response* resp, request_type type) {
    if(!resp) {
        return;
    }

    char message[strlen(resp->status_message) + 100];
    if(type == IMPORT_JOURNAL) {
        sprintf(message, "%s\n%s", journal_imported, resp->status_message);
        free(journal_imported);
    }
    else {
        sprintf(message, "%s\n%s", journal_saved, resp->status_message);
        free(journal_saved);
    }

    char command[100];
    sprintf(command, "/usr/bin/zenity --info --text=\"%s\"", message);
    popen(command, "r");
}


void print_controls() {
    printf("\nc - create journal\ni - import journal\ns - show journal\nd - delete journal\ne - exit");
    printf("\nc i [journal_nr]s [journal_nr]d [journal_nr]e\n");    
}


void print_menu() {
    system("clear");
    printf("Journals\n");

    for(size_t i = 0; i < journals_count; i++) {
        printf("%zu. %s\n", i + 1, journals[i]);
    }

    print_controls();
}



void add_new_journal(char* journal_name) {
    journals[journals_count] = (char*)malloc(strlen(journal_name) + 1);
    strcpy(journals[journals_count], journal_name);
    journals_count++;
}


void show_create_journal_screen() {
    system("clear");
    puts("Create journal\n");
    puts("Input journal name: ");

    char* journal_name = NULL;
    size_t journal_name_len;
    getline(&journal_name, &journal_name_len, stdin);
    if(strcmp(journal_name, "\n") == 0) {
        free(journal_name);
        puts("Invalid journal name. Returning to main menu...\n");
        sleep(1);
        return;
    }

    puts("Creating journal. Please wait...");
    response* resp = create_journal(journal_name);

    log_info("HWEEW RESPONSE");
    if(!resp) {
        sleep(1);
        return;
    }

    if(resp->status == SUCCESS) {
        add_new_journal(journal_name);
        free(journal_name);
    }   
    else {
        puts("Failed to create journal or something bad happened while creating it.");
        free(journal_name);
        sleep(1);
        return;
    }

    puts(resp->status_message);
    delete_response(resp);
    sleep(1);
}


int get_zip_content(char* file_path, char** buffer, size_t* size) {
    FILE *file = fopen(file_path, "rb");

    if (file == NULL) {
        log_error("Error opening selected zip file. Error: %s", strerror(errno));
        return -1;
    }

    log_debug("Opened zip file: %s\n", file_path);
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);

    *buffer = (char*)malloc(*size);
    if (buffer == NULL) {
        log_error("Error allocating memory. Error: %s", strerror(errno));
        fclose(file);
        return -1;
    }

    log_debug("Size %zu", *size);
    log_debug("Size %zu", *size);

    size_t result = fread(*buffer, 1, *size, file);
    log_debug("Size %zu", *size);

    if (!result) {
        log_error("Error reading file. Error: %s", strerror(errno));
        free(buffer);
        *size = 0;
        fclose(file);
        return -1;
    }

    log_debug("Size %zu", *size);
    fclose(file);
}


void show_import_journal_screen() {
    if(journal_imported ) {
        puts("A journal is already importing. Try again later.");
        sleep(1);
        return;
    }

    system("clear");
    puts("Import journal\n");
    puts("Input journal name: ");

    char* journal_name = NULL;
    size_t journal_name_size;
    getline(&journal_name, &journal_name_size, stdin);

    if(strcmp(journal_name, "\n") == 0) {
        free(journal_name);
        puts("Invalid journal name. Returning to main menu...\n");
        sleep(1);
        return;
    }

    puts("Choose journal location:\n");

    const char zenity_path[] = "/usr/bin/zenity";
    char file_select_command[150];

    sprintf(file_select_command, "%s --file-selection --modal --title=\"%s\" --file-filter=\"ZIP files | *.zip\"", zenity_path, "Select file");

    char journal_path[512];
    FILE* command_output = popen(file_select_command, "r");
    if(!command_output) {
        puts("Could not open file chooser. Returning to main menu...");
        free(journal_name);
        sleep(1);
        return;
    }

    fgets(journal_path, 512, command_output); 
    char* new_line = strstr(journal_path, "\n");
    if(new_line) {
        *new_line = '\0';
    }

    int res = pclose(command_output);
    if(res < 0) {
        log_error("Could not close zenity output file. Error: %s", strerror(errno));
    }

    log_debug("Chosen journal path: %s", journal_path);
    puts("Importing journal. You can continue your work until import is finished.");

    char* zip_content = NULL;
    size_t zip_content_size = -1;

    res = get_zip_content(journal_path, &zip_content, &zip_content_size);
    if(res == -1) {
        log_error("Import failed. Error: %s", strerror(errno));
        sleep(10);
        return;
    }

    log_debug("Zip loaded succesfully. %zu", zip_content_size);

    res = import_journal(journal_name, zip_content, zip_content_size);
    if(res == -1) {
        log_error("Import failed. Error: %s", strerror(errno));
        sleep(1);
        free(journal_name);
        free(zip_content);
        return;
    }

    log_info("HERER");
    journal_imported = (char*)malloc(strlen(journal_name) + 1);
    strcpy(journal_imported, journal_name);
    sleep(1);
}


void show_journal_screen(size_t journal_nr) {
    if(journal_saved && strcmp(journal_saved, journals[journal_nr])) {
        puts("Selected journal is in saving process. Try again later.");
        sleep(1);
        return;
    }

    system("clear");
    printf("Retrieving journal %s...\n", journals[journal_nr]);

    response* resp = retrieve_journal(journals[journal_nr]);
    if(!resp) {
        puts("Could not retrieve journal. Returning to main menu...\n");
        sleep(1);
        return;
    }

    if(resp->status == FAIL) {
        puts(resp->status_message);
        delete_response(resp);
        sleep(1);
        return;
    }

    char journal_path[512];
    sprintf(journal_path, "./.tmp/%s.zip", journals[journal_nr]);
    FILE* journal_zip = fopen("./.tmp/%s.zip", "w+");
    if(!journal_zip) {
        puts("Something bad happened. Returning to main menu...\n");
        delete_response(resp);
        sleep(1);
        return;
    }

    fwrite((void*)resp->data, 1, resp->data_size, journal_zip);
    fclose(journal_zip);

    struct stat file_stat;
    if (stat(journal_path, &file_stat) == -1) {
        log_error("Something bad happened. Returning to main menu...");
        delete_response(resp);
        sleep(1);
        return;
    }

    delete_response(resp);
    time_t last_modified = file_stat.st_mtime;

    system("clear");
    printf("Opening %s\n...", journals[journal_nr]);

    char command[600];
    sprintf(command, "/usr/bin/python3 ./journal_editor/editor.py --journal %s", journal_path);
    FILE* editor_output = popen(command, "r");
    if(!editor_output) {
        puts("Could not open journal editor. Returning to main menu...");
        sleep(1);
        return;
    }

    char exit_command[10];
    fgets(exit_command, 10, editor_output);
    if(strcmp(exit_command, "Exit\n") != 0) {
        puts("Something bad happened while trying to open journal editor. Returning to main menu...\n");
        sleep(1);
        return;
    }

    int res = pclose(editor_output);
    if(res < 0) {
        log_error("Could not close journal editor output file. Error: %s", strerror(errno));
    }

    puts("Saving changes... You can continue your work while journal is saved.");
    char* zip_content = NULL;
    size_t zip_content_size;
    res = get_zip_content(journal_path, &zip_content, &zip_content_size);

    if(res == -1) {
        log_error("Saving failed. Error: %s", strerror(errno));
        sleep(1);
        return;
    }

    struct stat new_file_stat;
    if (stat(journal_path, &new_file_stat) == -1) {
        log_error("Something bad happened. Returning to main menu...");
        sleep(1);
        return;
    }

    if(last_modified != new_file_stat.st_mtime) {
        res = modify_journal(journals[journal_nr], zip_content, zip_content_size);
        if(res == -1) {
            free(zip_content);
            sleep(1);
        }

        journal_saved = (char*)malloc(strlen(journals[journal_nr]) + 1);
        strcpy(journal_saved, journals[journal_nr]);
    }

    puts("Returning to main menu...");
    sleep(1);
}


void show_delete_journal_screen(size_t journal_nr) {
    printf("Deleting journal %s...", journals[journal_nr]);

    response* resp = delete_journal(journals[journal_nr]);
    if(!resp) {
        sleep(1);
        return;
    }

    if(resp->status == SUCCESS) {
        free(journals[journal_nr]);
        memmove((void*)journals + journal_nr, (void*)(journals + journal_nr + 1), (journals_count - journal_nr - 1) * sizeof(char*));
        journals_count--;
    }

    puts(resp->status_message);
    delete_response(resp);
    sleep(1);
}


void exit_app() {
    puts("Exitting...");
    sleep(1);
    exit(0);
}


void get_and_handle_choice() {
    char* choice_str = NULL;
    size_t choice_len;

    getline(&choice_str, &choice_len, stdin);

    int journal_nr = -1;
    char choice;
    int res = sscanf(choice_str, "%c\n", &choice);
    free(choice_str);

    if(res < 1) {
        int res = sscanf(choice_str, "%d%c\n", &journal_nr, &choice);
    }

    if(res < 1 || res == EOF) {
        puts("Invalid command. Try again. \n");
        sleep(1);
        return;
    }

    log_info("Chosen command: %c", choice);
    if(journal_nr == -1 && (choice == 's' || choice == 'd')) {
        puts("Invalid command. Try again.\n");
        sleep(1);
        return;
    }

    switch(choice)
    {
    case 'c':
        show_create_journal_screen();
        break;
    case 'i':
        show_import_journal_screen();
        break;
    case 's':
        show_journal_screen(journal_nr);
    case 'd':
        show_delete_journal_screen(journal_nr);
        break;
    case 'e':
        exit_app();
        break;
    default:
        puts("Invalid command. Try again.\n");
        sleep(1);
        break;
    }

    log_info("FINal handle");
}


void start_event_loop() {
    while(!STOP_CLIENT_FLAG) {
        get_and_handle_choice();
        print_menu();
    }
}


void init_interface() {
    register_notifications_callback(notif_callback);

    log_info("Fetching journals. Please wait...");
    response* resp = retrieve_journals();
    log_info("%s sad", resp->data);

    if(!resp || resp->status == FAIL || !resp->data) {
        log_info("There are no journals found. Create or import one.");
    }
    else {
        printf("Journals\n");

        journals = (char**)malloc(100 * sizeof(char*));
        char* p = strtok(resp->data, ";");
        while(p) {
            add_new_journal(p);
            printf("%zu. %s\n", journals_count, p);
            p = strtok(NULL, ";");
        }
    }

    print_controls();
    start_event_loop();
}