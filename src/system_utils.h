#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include "config.h"

// Function prototypes

// Checks if the current user is root
int check_root(void);

// Executes a shell command and logs the output.
int execute_command(const char *cmd, int log_output);

// Legacy function name for compatibility
int run_command(const char *cmd);

// Run command in chroot environment
int run_command_chroot(const char *rootfs_path, const char *cmd);

// Gets the number of CPU cores available on the system.
int get_cpu_count(void);
int get_cpu_cores(void);  // Alternative name for compatibility

// Creates a directory if it doesn't exist.
int create_directory_util(const char *path);
int create_directory(const char *path);  // Alternative name for compatibility

#endif // SYSTEM_UTILS_H
