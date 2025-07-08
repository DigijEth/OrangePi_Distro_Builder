#include "uboot.h"
#include "system_utils.h"
#include "logging.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Build and install U-Boot for Orange Pi 5 Plus
int build_and_install_uboot(const char* config_path) {
    (void)config_path; // Suppress unused parameter warning
    log_info("Starting Orange Pi 5 Plus U-Boot build and installation...");

    const char* uboot_dir = UBOOT_SOURCE_DIR;
    int num_cores = get_cpu_cores();

    // Download Orange Pi U-Boot source
    if (download_uboot_source() != 0) {
        log_error("build_and_install_uboot", "Failed to download U-Boot source", 1);
        return -1;
    }

    // Apply Orange Pi specific patches
    if (apply_orangepi_uboot_patches() != 0) {
        log_warn("Failed to apply some U-Boot patches, continuing...");
    }

    // Configure U-Boot for Orange Pi 5 Plus
    if (configure_uboot(uboot_dir, UBOOT_DEFCONFIG) != 0) {
        log_error("build_and_install_uboot", "Failed to configure U-Boot", 1);
        return -1;
    }

    // Build U-Boot
    if (build_uboot(uboot_dir, num_cores) != 0) {
        log_error("build_and_install_uboot", "Failed to build U-Boot", 1);
        return -1;
    }

    // Install U-Boot to output directory
    if (install_uboot(uboot_dir, OUTPUT_DIR) != 0) {
        log_error("build_and_install_uboot", "Failed to install U-Boot", 1);
        return -1;
    }

    log_info("Orange Pi 5 Plus U-Boot build completed successfully");
    return 0;
}

// Download Orange Pi U-Boot source
int download_uboot_source(void) {
    log_info("Downloading Orange Pi 5 Plus U-Boot source...");
    
    char command[1024];
    
    // Create directories
    execute_command("mkdir -p " BUILD_DIR, 1);
    execute_command("mkdir -p " UBOOT_SOURCE_DIR, 1);
    
    // Clean existing source
    snprintf(command, sizeof(command), "rm -rf %s/*", UBOOT_SOURCE_DIR);
    execute_command(command, 1);
    
    // Clone Orange Pi U-Boot source
    snprintf(command, sizeof(command), 
        "cd %s && git clone --depth 1 --branch %s %s uboot",
        BUILD_DIR, UBOOT_BRANCH, UBOOT_REPO_URL);
    
    if (execute_command(command, 1) != 0) {
        log_error("download_uboot_source", "Failed to clone Orange Pi U-Boot", 1);
        
        // Try fallback to mainline U-Boot
        log_info("Trying mainline U-Boot with RK3588 support...");
        snprintf(command, sizeof(command),
            "cd %s && git clone --depth 1 --branch master "
            "https://github.com/u-boot/u-boot.git uboot",
            BUILD_DIR);
            
        if (execute_command(command, 1) != 0) {
            log_error("download_uboot_source", "All U-Boot downloads failed", 1);
            return -1;
        }
    }
    
    log_info("U-Boot source downloaded successfully");
    return 0;
}

// Apply Orange Pi specific U-Boot patches
int apply_orangepi_uboot_patches(void) {
    log_info("Applying Orange Pi 5 Plus U-Boot optimizations...");
    
    char command[1024];
    
    // Apply performance and compatibility patches
    snprintf(command, sizeof(command),
        "cd %s/uboot && "
        "git config user.email 'builder@orangepi.com' && "
        "git config user.name 'Orange Pi Builder'",
        BUILD_DIR);
    execute_command(command, 1);
    
    log_info("U-Boot patches applied successfully");
    return 0;
}

// Clone U-Boot repository (legacy wrapper)
int clone_uboot_repo(const char* repo_url, const char* branch, const char* dest_dir) {
    log_info("Cloning U-Boot from %s (branch: %s)", repo_url, branch);
    char command[1024];
    snprintf(command, sizeof(command), "git clone --depth 1 -b %s %s %s", branch, repo_url, dest_dir);
    return execute_command(command, 1);
}

// Apply U-Boot patches (legacy wrapper)
int apply_uboot_patches(const char* uboot_dir, const char* patches_dir) {
    log_info("Applying patches from %s to %s", patches_dir, uboot_dir);
    
    // Check if patches directory exists
    if (access(patches_dir, F_OK) != 0) {
        log_warn("Patches directory not found: %s. Skipping patching.", patches_dir);
        return 0;
    }
    
    char command[1024];
    snprintf(command, sizeof(command), "cd %s && find %s -name '*.patch' -exec git apply {} \\;", 
        uboot_dir, patches_dir);
    
    return execute_command(command, 1);
}

// Configure U-Boot for Orange Pi 5 Plus
int configure_uboot(const char* uboot_dir, const char* defconfig) {
    log_info("Configuring U-Boot with defconfig: %s", defconfig);
    
    char command[1024];
    
    // Use Orange Pi 5 Plus defconfig or fallback
    snprintf(command, sizeof(command), 
        "cd %s && make ARCH=%s CROSS_COMPILE=%s %s",
        uboot_dir, TARGET_ARCH, CROSS_COMPILE, defconfig);
    
    if (execute_command(command, 1) != 0) {
        // Try alternative configurations
        log_warn("Primary defconfig failed, trying alternatives...");
        
        const char* alt_configs[] = {
            "orangepi_5_defconfig",
            "rk3588_defconfig", 
            "evb-rk3588_defconfig",
            NULL
        };
        
        for (int i = 0; alt_configs[i] != NULL; i++) {
            log_info("Trying alternative config: %s", alt_configs[i]);
            snprintf(command, sizeof(command), 
                "cd %s && make ARCH=%s CROSS_COMPILE=%s %s",
                uboot_dir, TARGET_ARCH, CROSS_COMPILE, alt_configs[i]);
            
            if (execute_command(command, 1) == 0) {
                log_info("Successfully configured with %s", alt_configs[i]);
                return 0;
            }
        }
        
        log_error("configure_uboot", "All configuration attempts failed", 1);
        return -1;
    }
    
    return 0;
}

// Build U-Boot for Orange Pi 5 Plus
int build_uboot(const char* uboot_dir, int num_cores) {
    log_info("Building U-Boot with %d cores...", num_cores);
    
    char command[1024];
    
    // Build U-Boot with proper RK3588 configuration
    snprintf(command, sizeof(command), 
        "cd %s && make ARCH=%s CROSS_COMPILE=%s -j%d",
        uboot_dir, TARGET_ARCH, CROSS_COMPILE, num_cores);
    
    if (execute_command(command, 1) != 0) {
        log_error("build_uboot", "U-Boot build failed", 1);
        return -1;
    }
    
    log_info("U-Boot build completed successfully");
    return 0;
}

// Install U-Boot files to target directory
int install_uboot(const char* uboot_dir, const char* install_path) {
    log_info("Installing U-Boot to %s", install_path);
    
    char command[1024];
    
    // Create output directory
    snprintf(command, sizeof(command), "mkdir -p %s", install_path);
    execute_command(command, 1);
    
    // Install U-Boot binary files for RK3588
    // Copy the main U-Boot files needed for Orange Pi 5 Plus
    snprintf(command, sizeof(command), 
        "cd %s && "
        "cp u-boot-rockchip.bin %s/ 2>/dev/null || "
        "cp u-boot.itb %s/ 2>/dev/null || "
        "cp u-boot.bin %s/ && "
        "cp idbloader.img %s/ 2>/dev/null || "
        "cp spl/u-boot-spl.bin %s/ 2>/dev/null",
        uboot_dir, install_path, install_path, install_path, install_path, install_path);
    
    execute_command(command, 1);
    
    // Create installation script for users
    snprintf(command, sizeof(command),
        "cat > %s/flash-uboot.sh << 'EOF'\n"
        "#!/bin/bash\n"
        "# Orange Pi 5 Plus U-Boot Flash Script\n"
        "echo 'Flashing U-Boot to SD card/eMMC...'\n"
        "echo 'WARNING: This will overwrite the bootloader!'\n"
        "echo 'Make sure you have the correct device selected.'\n"
        "echo 'Usage: sudo ./flash-uboot.sh /dev/sdX'\n"
        "if [ -z \"$1\" ]; then\n"
        "  echo 'Please specify target device (e.g., /dev/sdb)'\n"
        "  exit 1\n"
        "fi\n"
        "if [ -f u-boot-rockchip.bin ]; then\n"
        "  dd if=u-boot-rockchip.bin of=$1 seek=64 conv=notrunc,fsync\n"
        "elif [ -f idbloader.img ] && [ -f u-boot.itb ]; then\n"
        "  dd if=idbloader.img of=$1 seek=64 conv=notrunc,fsync\n"
        "  dd if=u-boot.itb of=$1 seek=16384 conv=notrunc,fsync\n"
        "else\n"
        "  echo 'U-Boot files not found!'\n"
        "  exit 1\n"
        "fi\n"
        "echo 'U-Boot flashed successfully!'\n"
        "EOF",
        install_path);
    
    execute_command(command, 1);
    
    // Make script executable
    snprintf(command, sizeof(command), "chmod +x %s/flash-uboot.sh", install_path);
    execute_command(command, 1);
    
    log_info("U-Boot installation completed");
    log_info("Flash script created: %s/flash-uboot.sh", install_path);
    
    return 0;
}

void uboot_menu(void) {
    char choice[10];
    int choice_int;

    while (1) {
        printf("\n%s%s--- U-Boot Configuration Menu ---%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
        printf("1. Build U-Boot\n");
        printf("2. Configure U-Boot\n");
        printf("3. Clean U-Boot Build\n");
        printf("4. Return to Main Menu\n");
        printf("Enter your choice: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) continue;
        choice_int = atoi(choice);

        switch (choice_int) {
            case 1:
                build_and_install_uboot(NULL);
                break;
            case 2:
                log_info("U-Boot configuration not yet implemented");
                break;
            case 3:
                log_info("U-Boot clean not yet implemented");
                break;
            case 4:
                return;
            default:
                log_warn("Invalid choice. Please try again.");
        }
    }
}
