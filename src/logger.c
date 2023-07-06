#include "logger.h"

#include <stdio.h>
#include <time.h>


enum LOG_LEVEL {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};


static char* LOG_LEVEL_STRING[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR"
}


inline void logger(char* message, LOG_LEVEL level) {
    time_t time = time(NULL);
    struct tm current_time = *localtime(&t);
    FILE* output = (level == WARNING || level == ERROR) ? stderr : stdout;

    fprintf(output, "%d-%02d-%02d %02d:%02d:%02d %s: %s", current_time.tm_year + 1900, 
        current_time.tm_mon + 1, current_time.tm_mday, current_time.tm_hour, current_time.tm_min, 
        current_time.tm_sec, LOG_LEVEL_STRING[level], message);
}


inline void log_debug(char* debug) {
    logger(debug, DEBUG);
}


inline void log_info(char* info) {
    logger(info, INFO);
}


inline void log_warning(char* warning) {
    logger(warning, WARNING);
}


inline void log_error(char* error) {
    logger(error, ERROR);
}
