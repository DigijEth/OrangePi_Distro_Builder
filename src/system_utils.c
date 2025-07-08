#include "system_utils.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CMD_LEN 2048
// LOG_FILE is defined in config.h

// Execute command with enhanced logging
int execute_command(const char *cmd, int show_output) {
    char log_cmd[MAX_CMD_LEN + 100];
    int result;

    if (!cmd || strlen(cmd) == 0) {
        log_error("execute_command", "Command string is empty or NULL", 0);
        return -1;
    }

    if (show_output) {
        printf("%s%s%s\n", COLOR_BLUE, cmd, COLOR_RESET);
        snprintf(log_cmd, sizeof(log_cmd), "%s 2>&1 | tee -a %s", cmd, LOG_FILE);
    } else {
        snprintf(log_cmd, sizeof(log_cmd), "%s >> %s 2>&1", cmd, LOG_FILE);
    }

    result = system(log_cmd);

    if (result != 0) {
        int exit_status = WEXITSTATUS(result);
        if (WIFSIGNALED(result)) {
            int signal_num = WTERMSIG(result);
            log_error("execute_command", "Command terminated by signal", signal_num);
        } else {
            log_error("execute_command", "Command failed with exit status", exit_status);
        }

        // Log the actual command that failed
        char error_msg[MAX_CMD_LEN + 100];
        snprintf(error_msg, sizeof(error_msg), "Failed command: %s", cmd);
        log_message("ERROR", error_msg);

        return -1;
    }

    return 0;
}

// Create directory if it doesn't exist
int create_directory(const char *path) {
    struct stat st = {0};

    if (!path) {
        log_error("create_directory", "Path is NULL", 0);
        return -1;
    }

    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) != 0) {
            log_system_error("create_directory", "mkdir");
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "Failed to create directory: %s", path);
            log_message("ERROR", error_msg);
            return -1;
        }
        log_message("INFO", "Created directory successfully");
    } else {
        if (S_ISDIR(st.st_mode)) {
            char info_msg[512];
            snprintf(info_msg, sizeof(info_msg), "Directory already exists: %s", path);
            log_message("INFO", info_msg);
        } else {
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), "Path exists but is not a directory: %s", path);
            log_error("create_directory", error_msg, 0);
            return -1;
        }
    }
    return 0;
}

// Get the number of CPU cores
int get_cpu_count(void) {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

// Check if running as root
int check_root(void) {
    if (getuid() != 0) {
        return -1; // Not root
    }
    return 0; // Is root
}

// Legacy function name for compatibility
int run_command(const char *cmd) {
    return execute_command(cmd, 1);
}

// Alternative name for compatibility
int get_cpu_cores(void) {
    return get_cpu_count();
}

// Run command in chroot environment
int run_command_chroot(const char *rootfs_path, const char *cmd) {
    char chroot_cmd[MAX_CMD_LEN + 512];
    snprintf(chroot_cmd, sizeof(chroot_cmd), "chroot %s /bin/bash -c '%s'", rootfs_path, cmd);
    return execute_command(chroot_cmd, 1);
}
