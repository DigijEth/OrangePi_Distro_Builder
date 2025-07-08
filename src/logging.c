#include "logging.h"
#include "config.h"
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

FILE *log_fp = NULL;

void init_logging(void) {
    log_fp = fopen(LOG_FILE, "a");
    if (!log_fp) {
        printf("Warning: Could not open log file %s for writing.\n", LOG_FILE);
    }
}

void log_message(const char *level, const char *message) {
    time_t now;
    char *timestamp;
    
    time(&now);
    timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline
    
    printf("[%s%s%s] %s%s%s\n", 
           COLOR_CYAN, timestamp, COLOR_RESET,
           strcmp(level, "ERROR") == 0 ? COLOR_RED : 
           strcmp(level, "SUCCESS") == 0 ? COLOR_GREEN :
           strcmp(level, "WARNING") == 0 ? COLOR_YELLOW : COLOR_RESET,
           message, COLOR_RESET);
    
    if (log_fp) {
        fprintf(log_fp, "[%s] [%s] %s\n", timestamp, level, message);
        fflush(log_fp);
    }
}

void log_error(const char *function, const char *message, int error_code) {
    char error_msg[1024];
    time_t now;
    char *timestamp;
    
    time(&now);
    timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline
    
    if (error_code != 0) {
        snprintf(error_msg, sizeof(error_msg), "[%s] %s (Error code: %d)", 
                 function, message, error_code);
    } else {
        snprintf(error_msg, sizeof(error_msg), "[%s] %s", function, message);
    }
    
    printf("[%s%s%s] %sERROR: %s%s\n", 
           COLOR_CYAN, timestamp, COLOR_RESET,
           COLOR_RED, error_msg, COLOR_RESET);
    
    if (log_fp) {
        fprintf(log_fp, "[%s] [ERROR] %s\n", timestamp, error_msg);
        fflush(log_fp);
    }
}

void log_system_error(const char *function, const char *operation) {
    char error_msg[1024];
    time_t now;
    char *timestamp;
    int err = errno;
    
    time(&now);
    timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline
    
    snprintf(error_msg, sizeof(error_msg), "[%s] %s failed: %s (errno: %d)", 
             function, operation, strerror(err), err);
    
    printf("[%s%s%s] %sERROR: %s%s\n", 
           COLOR_CYAN, timestamp, COLOR_RESET,
           COLOR_RED, error_msg, COLOR_RESET);
    
    if (log_fp) {
        fprintf(log_fp, "[%s] [ERROR] %s\n", timestamp, error_msg);
        fflush(log_fp);
    }
}

void log_info(const char *message, ...) {
    char formatted_message[1024];
    va_list args;
    
    va_start(args, message);
    vsnprintf(formatted_message, sizeof(formatted_message), message, args);
    va_end(args);
    
    log_message("INFO", formatted_message);
}

void log_warn(const char *message, ...) {
    char formatted_message[1024];
    va_list args;
    
    va_start(args, message);
    vsnprintf(formatted_message, sizeof(formatted_message), message, args);
    va_end(args);
    
    log_message("WARNING", formatted_message);
}
