#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>


static FILE* log_file;

void logger(char* level, char* message_format, va_list args) {
    time_t current_time = time(NULL);
    struct tm local_time = *localtime(&current_time);
    FILE* output;
    char log_message[100000];
    
    if(strcmp(level, "WARNING") == 0 || strcmp(level, "ERROR") == 0) {
        output = stderr;
    }
    else {
        output = stdout;
    }

    vsnprintf(log_message, 100000, message_format, args);
    fprintf(output, "%s\n", log_message);

    if(log_file) {
        fprintf(log_file, "%d-%02d-%02d %02d:%02d:%02d %s: %s\n", local_time.tm_year + 1900,
            local_time.tm_mon + 1, local_time.tm_mday, local_time.tm_hour, local_time.tm_min,
            local_time.tm_sec, level, log_message); 
        fflush(log_file);
    }
    
}


void log_debug(char* message_format, ...) {
    va_list args;
    va_start(args, message_format);
    logger("DEBUG", message_format, args);
    va_end(args);
}


void log_info(char* message_format, ...) {
    va_list args;
    va_start(args, message_format);
    logger("INFO", message_format, args);
    va_end(args);
}


void log_warning(char* message_format, ...) {
    va_list args;
    va_start(args, message_format);
    logger("WARNING", message_format, args);
    va_end(args);
}


void log_error(char* message_format, ...) {
    va_list args;
    va_start(args, message_format);
    logger("ERROR", message_format, args);
    va_end(args);
}


void initialize_log_file(char* file_name) {
    log_file = fopen(file_name, "w+");
    if(!log_file) {
        log_warning("Log file could not be opened. Error: %s", strerror(errno));
    }
}


void close_log_file() {
    fclose(log_file);
}
