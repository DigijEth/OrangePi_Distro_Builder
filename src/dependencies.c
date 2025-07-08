#include "dependencies.h"
#include "logging.h"
#include "system_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Install build prerequisites
int install_prerequisites(void) {
    log_message("INFO", "Installing build prerequisites...");

    const char *packages[] = {
        // Basic build tools
        "build-essential",
        "gcc-aarch64-linux-gnu",
        "g++-aarch64-linux-gnu",
        "libncurses-dev",
        "gawk",
        "flex",
        "bison",
        "openssl",
        "libssl-dev",
        "dkms",
        "libelf-dev",
        "libudev-dev",
        "libpci-dev",
        "libiberty-dev",
        "autoconf",
        "llvm",
        // Additional tools
        "git",
        "wget",
        "curl",
        "bc",
        "rsync",
        "kmod",
        "cpio",
        "python3",
        "python3-pip",
        "device-tree-compiler",
        // Ubuntu kernel build dependencies
        "fakeroot",
        "kernel-package",
        "pkg-config-dbgsym",
        // Mali GPU and OpenCL/Vulkan support
        "mesa-opencl-icd",
        "vulkan-tools",
        "vulkan-utils",
        "vulkan-validationlayers",
        "libvulkan-dev",
        "ocl-icd-opencl-dev",
        "opencl-headers",
        "clinfo",
        // Media and hardware acceleration
        "va-driver-all",
        "vdpau-driver-all",
        "mesa-va-drivers",
        "mesa-vdpau-drivers",
        // Development libraries
        "libegl1-mesa-dev",
        "libgles2-mesa-dev",
        "libgl1-mesa-dev",
        "libdrm-dev",
        "libgbm-dev",
        "libwayland-dev",
        "libx11-dev",
        "meson",
        "ninja-build",
        NULL
    };

    char cmd[MAX_CMD_LEN * 2];
    int i;

    // Install all packages at once
    strcpy(cmd, "DEBIAN_FRONTEND=noninteractive apt install -y");
    for (i = 0; packages[i] != NULL; i++) {
        strcat(cmd, " ");
        strcat(cmd, packages[i]);
    }

    if (execute_command(cmd, 1) != 0) {
        log_message("ERROR", "Failed to install prerequisites");
        return -1;
    }

    // Install additional Ubuntu kernel build dependencies
    if (execute_command("apt build-dep -y linux linux-image-unsigned-$(uname -r)", 1) != 0) {
        log_message("WARNING", "Failed to install some kernel build dependencies");
    }

    log_message("SUCCESS", "Prerequisites installed successfully");
    return 0;
}

int check_dependencies(void) {
    // This function is now more of a placeholder, as the interactive menu handles dependency checks.
    // We can add more robust checks here in the future.
    log_message("INFO", "Checking for essential build tools...");
    if (system("command -v gcc > /dev/null 2>&1") != 0) {
        log_message("WARNING", "GCC not found. Please install build-essential.");
        return -1;
    }
    if (system("command -v git > /dev/null 2>&1") != 0) {
        log_message("WARNING", "Git not found. Please install git.");
        return -1;
    }
    log_message("SUCCESS", "Essential tools found.");
    return 0;
}

void dependencies_menu(void) {
    char choice[10];
    int choice_int;

    while (1) {
        printf("\n%s%s--- Dependencies Menu ---%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
        printf("1. Check Dependencies\n");
        printf("2. Install Prerequisites\n");
        printf("3. Return to Main Menu\n");
        printf("Enter your choice: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) continue;
        choice_int = atoi(choice);

        switch (choice_int) {
            case 1:
                if (check_dependencies() == 0) {
                    log_message("SUCCESS", "All dependencies are satisfied.");
                } else {
                    log_message("WARNING", "Some dependencies are missing.");
                }
                break;
            case 2:
                if (install_prerequisites() == 0) {
                    log_message("SUCCESS", "Prerequisites installed successfully.");
                } else {
                    log_message("ERROR", "Failed to install prerequisites.");
                }
                break;
            case 3:
                return;
            default:
                log_message("WARNING", "Invalid choice. Please try again.");
        }
    }
}
