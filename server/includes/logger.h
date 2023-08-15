#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>


FILE* log_file = NULL;


#define LOGGER(level, message_format, ...) time_t current_time = time(NULL); \
    struct tm local_time = *localtime(&current_time); \
    FILE* output = (strcmp(level, "WARNING") == 0 || strcmp(level, "ERROR") == 0) ? stderr : stdout; \
    char log_message[100000]; \
    snprintf(log_message, 100000, message_format, __VA_ARGS__); \
    fprintf(output, "%d-%02d-%02d %02d:%02d:%02d %s: %s\n", local_time.tm_year + 1900, \
        local_time.tm_mon + 1, local_time.tm_mday, local_time.tm_hour, local_time.tm_min, \
        local_time.tm_sec, level, log_message); \
    if(log_file) { \
        fprintf(output, "%d-%02d-%02d %02d:%02d:%02d %s: %s", local_time.tm_year + 1900, \
            local_time.tm_mon + 1, local_time.tm_mday, local_time.tm_hour, local_time.tm_min, \
            local_time.tm_sec, level, log_message); \
    }


#define LOG_DEBUG(message_format, ...) LOGGER("DEBUG", message_format, __VA_ARGS__)

#define LOG_INFO(message_format, ...) LOGGER("INFO", message_format, __VA_ARGS__)

#define LOG_WARNING(message_format, ...) LOGGER("WARNING", message_format, __VA_ARGS__)

#define LOG_ERROR(message_format, ...) LOGGER("ERROR", message_format, __VA_ARGS__)

void initialize_log_file(char* file_name) {
    log_file = fopen(file_name, "w+");
    if(!log_file) {
        LOG_WARNING("Log file could not be opened. Error: %s", strerror(errno));
    }
}

void close_log_file() {
    fclose(log_file);
}


#endif // LOGGER_H