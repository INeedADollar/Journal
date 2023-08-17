#ifndef LOGGER_H
#define LOGGER_H


void log_debug(char* message_format, ...);

void log_info(char* message_format, ...);

void log_warning(char* message_format, ...);

void log_error(char* message_format, ...);

void initialize_log_file(char* file_name);

void close_log_file();


#endif // LOGGER_H