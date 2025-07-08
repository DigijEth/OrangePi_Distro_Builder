#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

// Global log file pointer
extern FILE *log_fp;

// Function declarations
void init_logging(void);
void log_message(const char *level, const char *message);
void log_info(const char *message, ...);
void log_warn(const char *message, ...);
void log_error(const char *function, const char *message, int error_code);
void log_system_error(const char *function, const char *operation);

#endif // LOGGING_H
