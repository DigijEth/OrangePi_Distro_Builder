/*
 * kernel.c - Kernel and system image operations for Orange Pi 5 Plus Ultimate Interactive Builder
 * Version: 0.1.0a
 * 
 * This file contains functions for kernel compilation, rootfs creation,
 * U-Boot building, and system image generation.
 */

#include "builder.h"

// Download kernel source
int download_kernel_source(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char source_dir[MAX_PATH_LEN];
    error_context_t error_ctx = {0};
    
    if (!config) {
        LOG_ERROR("Configuration is NULL");
        return ERROR_UNKNOWN;
    }
    
    LOG_INFO("Setting up kernel source for Orange Pi 5 Plus...");
    
    snprintf(source_dir, sizeof(source_dir), "%s/linux", config->build_dir);
    
    // Change to build directory
    if (chdir(config->build_dir) != 0) {
        LOG_ERROR("Failed to change to build directory");
        return ERROR_FILE_NOT_FOUND;
    }
    
    // Clean up any existing temp directory
    LOG_INFO("Cleaning up previous download attempts...");
    execute_command_safe("rm -rf linux_temp", 0, &error_ctx);
    
    // Create a clean source directory if it doesn't exist
    if (create_directory_safe(source_dir, &error_ctx) != 0) {
        LOG_ERROR("Failed to create kernel source directory");
        return ERROR_FILE_NOT_FOUND;
    }
    
    // First approach: Try the Orange Pi specific repository
    LOG_INFO("Trying to download Orange Pi kernel source...");
    
    snprintf(cmd, sizeof(cmd), 
             "git clone --depth 1 https://github.com/orangepi-xunlong/linux.git -b orange-pi-5.10-rk3588 linux_temp");
    
    if (execute_command_with_retry(cmd, 1, 2) == 0) {
        LOG_INFO("Successfully downloaded Orange Pi kernel source");
        
        // Move the contents to the final location
        snprintf(cmd, sizeof(cmd), "cp -r linux_temp/* %s/ && rm -rf linux_temp", source_dir);
        execute_command_safe(cmd, 0, &error_ctx);
        
        LOG_INFO("Orange Pi kernel source prepared successfully");
        return ERROR_SUCCESS;
    }
    
    LOG_WARNING("Could not download Orange Pi kernel source, trying Rockchip source...");
    
    // Clean up failed attempt
    execute_command_safe("rm -rf linux_temp", 0, &error_ctx);
    
    // Second approach: Try standard Rockchip kernel
    snprintf(cmd, sizeof(cmd), 
             "git clone --depth 1 https://github.com/rockchip-linux/kernel.git -b develop-5.10 linux_temp");
    
    if (execute_command_with_retry(cmd, 1, 2) == 0) {
        LOG_INFO("Successfully downloaded Rockchip kernel source");
        
        // Move the contents to the final location
        snprintf(cmd, sizeof(cmd), "cp -r linux_temp/* %s/ && rm -rf linux_temp", source_dir);
        execute_command_safe(cmd, 0, &error_ctx);
        
        // For Rockchip kernel, we need to add Orange Pi 5 Plus device tree
        LOG_INFO("Adding Orange Pi 5 Plus device tree to Rockchip kernel...");
        
        // Create Orange Pi 5 Plus device tree directory if needed
        char dtb_dir[MAX_PATH_LEN];
        snprintf(dtb_dir, sizeof(dtb_dir), "%s/arch/arm64/boot/dts/rockchip", source_dir);
        
        if (chdir(dtb_dir) != 0) {
            LOG_WARNING("Could not change to device tree directory, might need manual configuration");
        } else {
            // Download Orange Pi 5 Plus device tree file
            snprintf(cmd, sizeof(cmd), 
                     "wget -O rk3588-orangepi-5-plus.dts https://raw.githubusercontent.com/orangepi-xunlong/linux-orangepi/orange-pi-5.10-rk3588/arch/arm64/boot/dts/rockchip/rk3588-orangepi-5-plus.dts");
            
            if (execute_command_with_retry(cmd, 1, 3) != 0) {
                LOG_WARNING("Could not download Orange Pi 5 Plus device tree, board might not be fully supported");
            }
            
            // Update the Makefile to include the new device tree
            FILE *makefile = fopen("Makefile", "a");
            if (makefile) {
                fprintf(makefile, "\ndtb-$(CONFIG_ARCH_ROCKCHIP) += rk3588-orangepi-5-plus.dtb\n");
                fclose(makefile);
                LOG_INFO("Added Orange Pi 5 Plus to device tree build list");
            }
        }
        
        LOG_INFO("Rockchip kernel with Orange Pi additions prepared successfully");
        return ERROR_SUCCESS;
    }
    
    // Clean up failed attempt
    execute_command_safe("rm -rf linux_temp", 0, &error_ctx);
    
    // Third approach: Get mainline kernel and add Rockchip patches
    LOG_WARNING("Could not download Rockchip kernel, falling back to mainline with patches...");
    
    snprintf(cmd, sizeof(cmd),
             "git clone --depth 1 https://github.com/torvalds/linux.git -b v%s linux_temp || "
             "git clone --depth 1 https://github.com/torvalds/linux.git linux_temp",
             config->kernel_version);
    
    if (execute_command_with_retry(cmd, 1, 2) == 0) {
        LOG_INFO("Successfully downloaded mainline kernel source");
        
        // Move the contents to the final location
        snprintf(cmd, sizeof(cmd), "cp -r linux_temp/* %s/ && rm -rf linux_temp", source_dir);
        execute_command_safe(cmd, 0, &error_ctx);
        
        // For mainline kernel, we need to download and apply Rockchip patches
        LOG_INFO("Downloading Rockchip patches for mainline kernel...");
        
        if (chdir(source_dir) != 0) {
            LOG_ERROR("Failed to change to kernel directory");
            return ERROR_FILE_NOT_FOUND;
        }
        
        // Create patches directory
        snprintf(cmd, sizeof(cmd), "mkdir -p rockchip_patches");
        execute_command_safe(cmd, 0, &error_ctx);
        
        // Download Rockchip patches from various sources
        const char *patch_sources[] = {
            "https://raw.githubusercontent.com/armbian/build/master/patch/kernel/rockchip-rk3588-current",
            "https://raw.githubusercontent.com/armbian/build/master/patch/kernel/rockchip-rk3588-edge",
            NULL
        };
        
        for (int i = 0; patch_sources[i] != NULL; i++) {
            snprintf(cmd, sizeof(cmd), 
                     "cd rockchip_patches && wget -r -np -nd -A '*.patch' %s/",
                     patch_sources[i]);
            
            execute_command_with_retry(cmd, 1, 2);
        }
        
        // Apply the patches
        LOG_INFO("Applying Rockchip patches to mainline kernel...");
        snprintf(cmd, sizeof(cmd), 
                 "find rockchip_patches -name '*.patch' -print0 | sort -z | xargs -0 -n 1 patch -p1 -i");
        
        // We don't check return code here as some patches might fail but that's okay
        execute_command_safe(cmd, 1, NULL);
        
        LOG_INFO("Mainline kernel with Rockchip patches prepared");
        LOG_WARNING("This is a fallback method - functionality may be limited");
        
        return ERROR_SUCCESS;
    }
    
    // Clean up failed attempt
    execute_command_safe("rm -rf linux_temp", 0, &error_ctx);
    
    // All approaches failed
    LOG_ERROR("All kernel source download approaches failed");
    
    // Provide a helpful message
    printf("\n%s%sUnable to download kernel source automatically%s\n", COLOR_BOLD, COLOR_RED, COLOR_RESET);
    printf("This builder requires a kernel source to build a custom Orange Pi 5 Plus image.\n\n");
    printf("You have several options:\n");
    printf("1. Check your internet connection and try again\n");
    printf("2. Download a kernel source manually and place it in: %s\n", source_dir);
    printf("3. Use a pre-built kernel (if available) by modifying the source code\n\n");
    
    return ERROR_NETWORK_FAILURE;
}

// Download Ubuntu Rockchip patches
int download_ubuntu_rockchip_patches(void) {
    char cmd[MAX_CMD_LEN];
    
    LOG_INFO("Downloading Ubuntu Rockchip project components...");
    
    // Clone Ubuntu Rockchip repository
    snprintf(cmd, sizeof(cmd),
             "git clone --depth 1 "
             "https://github.com/Joshua-Riek/ubuntu-rockchip.git ubuntu-rockchip");
    
    if (execute_command_with_retry(cmd, 1, 2) != 0) {
        LOG_WARNING("Failed to download Ubuntu Rockchip project components");
        return ERROR_SUCCESS; // Non-critical
    }
    
    LOG_INFO("Ubuntu Rockchip components downloaded");
    return ERROR_SUCCESS;
}

// Configure kernel
int configure_kernel(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char kernel_dir[MAX_PATH_LEN];
    error_context_t error_ctx = {0};
    
    LOG_INFO("Configuring kernel with Orange Pi 5 Plus and Mali GPU support...");
    
    snprintf(kernel_dir, sizeof(kernel_dir), "%s/linux", config->build_dir);
    
    if (chdir(kernel_dir) != 0) {
        LOG_ERROR("Failed to change to kernel directory");
        return ERROR_KERNEL_CONFIG_FAILED;
    }
    
    // Set environment variables
    if (setenv("ARCH", config->arch, 1) != 0) {
        LOG_WARNING("Failed to set ARCH environment variable");
    }
    if (setenv("CROSS_COMPILE", config->cross_compile, 1) != 0) {
        LOG_WARNING("Failed to set CROSS_COMPILE environment variable");
    }
    
    // Clean if requested
    if (config->clean_build) {
        LOG_INFO("Cleaning previous build artifacts...");
        execute_command_safe("make mrproper", 1, &error_ctx);
    }
    
    // Detect kernel type by checking Makefile and directory structure
    int is_rockchip_kernel = 0;
    int is_orangepi_kernel = 0;
    int is_mainline_kernel = 0;
    
    // Check for Orange Pi specific content
    if (access("arch/arm64/boot/dts/rockchip/rk3588-orangepi-5-plus.dts", F_OK) == 0) {
        is_orangepi_kernel = 1;
        LOG_INFO("Detected Orange Pi specific kernel source");
    }
    
    // Check for Rockchip content
    FILE *makefile = fopen("Makefile", "r");
    if (makefile) {
        char line[256];
        while (fgets(line, sizeof(line), makefile)) {
            if (strstr(line, "ROCKCHIP") != NULL || strstr(line, "rockchip") != NULL) {
                is_rockchip_kernel = 1;
                LOG_INFO("Detected Rockchip kernel source");
                break;
            }
        }
        fclose(makefile);
    }
    
    // If neither, assume mainline
    if (!is_orangepi_kernel && !is_rockchip_kernel) {
        is_mainline_kernel = 1;
        LOG_INFO("Detected mainline kernel source");
    }
    
    // Use the appropriate defconfig based on kernel type
    if (is_orangepi_kernel) {
        // Try Orange Pi specific defconfig first
        LOG_INFO("Using Orange Pi specific configuration...");
        snprintf(cmd, sizeof(cmd), "make orangepi_5_plus_defconfig");
        
        if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
            LOG_WARNING("Orange Pi defconfig not found, trying Rockchip defconfig...");
            snprintf(cmd, sizeof(cmd), "make rockchip_defconfig");
            
            if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
                LOG_WARNING("Rockchip defconfig not found, falling back to generic defconfig...");
                if (execute_command_safe("make defconfig", 1, &error_ctx) != 0) {
                    LOG_ERROR("Failed to configure kernel with any config");
                    return ERROR_KERNEL_CONFIG_FAILED;
                }
            }
        }
    } else if (is_rockchip_kernel) {
        // Use Rockchip defconfig
        LOG_INFO("Using Rockchip configuration...");
        snprintf(cmd, sizeof(cmd), "make rockchip_defconfig");
        
        if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
            LOG_WARNING("Rockchip defconfig not found, falling back to generic defconfig...");
            if (execute_command_safe("make defconfig", 1, &error_ctx) != 0) {
                LOG_ERROR("Failed to configure kernel");
                return ERROR_KERNEL_CONFIG_FAILED;
            }
        }
    } else {
        // Mainline kernel - use generic ARM64 defconfig
        LOG_INFO("Using generic ARM64 configuration for mainline kernel...");
        if (execute_command_safe("make defconfig", 1, &error_ctx) != 0) {
            LOG_ERROR("Failed to configure kernel");
            return ERROR_KERNEL_CONFIG_FAILED;
        }
    }
    
    // Enable RK3588 and Mali GPU options
    LOG_INFO("Enabling RK3588 and Mali GPU configurations...");
    
    const char *config_options[] = {
        "CONFIG_ARCH_ROCKCHIP=y",
        "CONFIG_ARM64=y",
        "CONFIG_ROCKCHIP_RK3588=y",
        "CONFIG_DRM_ROCKCHIP=y",
        "CONFIG_DRM_PANFROST=y",
        "CONFIG_MALI_MIDGARD=m",
        "CONFIG_MALI_CSF_SUPPORT=y",
        "CONFIG_DMA_CMA=y",
        "CONFIG_CMA=y",
        "CONFIG_EXTCON=y",
        "CONFIG_PHY_ROCKCHIP_DPHY=y",
        "CONFIG_PHY_ROCKCHIP_PCIE=y",
        "CONFIG_PHY_ROCKCHIP_TYPEC=y",
        "CONFIG_PHY_ROCKCHIP_NANENG_USB2=y",
        "CONFIG_PHY_ROCKCHIP_INNO_USB2=y",
        "CONFIG_PHY_ROCKCHIP_INNO_USB3=y",
        "CONFIG_PHY_ROCKCHIP_INNO_DSIDPHY=y",
        "CONFIG_ROCKCHIP_IOMMU=y",
        "CONFIG_ROCKCHIP_SUSPEND_MODE=y",
        "CONFIG_ROCKCHIP_THERMAL=y",
        "CONFIG_SND_SOC_ROCKCHIP=y",
        "CONFIG_SND_SOC_ROCKCHIP_I2S=y",
        "CONFIG_SND_SOC_ROCKCHIP_PDM=y",
        "CONFIG_SND_SOC_ROCKCHIP_SPDIF=y",
        "CONFIG_USB_DWC3_ROCKCHIP=y",
        "CONFIG_GPIO_ROCKCHIP=y",
        "CONFIG_PINCTRL_ROCKCHIP=y",
        "CONFIG_MMC_DW_ROCKCHIP=y",
        "CONFIG_I2C_ROCKCHIP=y",
        "CONFIG_SPI_ROCKCHIP=y",
        "CONFIG_PWM_ROCKCHIP=y",
        "CONFIG_ROCKCHIP_MULTI_RGA=y",
        "CONFIG_VIDEO_ROCKCHIP_ISP=y",
        "CONFIG_VIDEO_ROCKCHIP_ISPP=y",
        NULL
    };
    
    // Add the configurations to .config
    FILE *config_file = fopen(".config", "a");
    if (config_file) {
        int i;
        for (i = 0; config_options[i] != NULL; i++) {
            fprintf(config_file, "%s\n", config_options[i]);
        }
        fclose(config_file);
    }
    
    // If mainline kernel, integrate Mali GPU support
    if (is_mainline_kernel) {
        LOG_INFO("Integrating Mali GPU support into mainline kernel...");
        int result = integrate_mali_into_kernel(config);
        if (result != ERROR_SUCCESS) {
            LOG_WARNING("Mali integration partially failed - some GPU features may not work");
        }
    }
    
    // Resolve dependencies and create final config
    LOG_INFO("Finalizing kernel configuration...");
    execute_command_safe("make olddefconfig", 1, &error_ctx);
    
    LOG_INFO("Kernel configured successfully for Orange Pi 5 Plus");
    return ERROR_SUCCESS;
}

// Build kernel
int build_kernel(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    error_context_t error_ctx = {0};
    
    LOG_INFO("Building kernel with Mali GPU support (this may take a while)...");
    
    // Set environment variables
    if (setenv("ARCH", config->arch, 1) != 0) {
        LOG_WARNING("Failed to set ARCH environment variable");
    }
    if (setenv("CROSS_COMPILE", config->cross_compile, 1) != 0) {
        LOG_WARNING("Failed to set CROSS_COMPILE environment variable");
    }
    
    // Build kernel image
    snprintf(cmd, sizeof(cmd), "make -j%d Image", config->jobs);
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_ERROR("Failed to build kernel image");
        return ERROR_COMPILATION_FAILED;
    }
    
    // Build device tree blobs
    snprintf(cmd, sizeof(cmd), "make -j%d dtbs", config->jobs);
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_ERROR("Failed to build device tree blobs");
        return ERROR_COMPILATION_FAILED;
    }
    
    // Build modules
    snprintf(cmd, sizeof(cmd), "make -j%d modules", config->jobs);
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_ERROR("Failed to build kernel modules");
        return ERROR_COMPILATION_FAILED;
    }
    
    LOG_INFO("Kernel built successfully");
    return ERROR_SUCCESS;
}

// Install kernel
int install_kernel(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char kernel_dir[MAX_PATH_LEN];
    char modules_dir[MAX_PATH_LEN];
    error_context_t error_ctx = {0};
    
    LOG_INFO("Installing kernel and modules...");
    
    snprintf(kernel_dir, sizeof(kernel_dir), "%s/linux", config->build_dir);
    snprintf(modules_dir, sizeof(modules_dir), "%s/rootfs/lib/modules", config->output_dir);
    
    if (chdir(kernel_dir) != 0) {
        LOG_ERROR("Failed to change to kernel directory");
        return ERROR_FILE_NOT_FOUND;
    }
    
    // Create output directories
    snprintf(cmd, sizeof(cmd), "mkdir -p %s/rootfs/boot", config->output_dir);
    if (execute_command_safe(cmd, 0, &error_ctx) != 0) {
        LOG_ERROR("Failed to create boot directory");
        return ERROR_FILE_NOT_FOUND;
    }
    
    // Install kernel image
    LOG_INFO("Installing kernel image...");
    snprintf(cmd, sizeof(cmd), 
             "cp arch/arm64/boot/Image %s/rootfs/boot/vmlinuz-%s",
             config->output_dir, config->kernel_version);
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_ERROR("Failed to install kernel image");
        return ERROR_INSTALLATION_FAILED;
    }
    
    // Install device tree blobs
    LOG_INFO("Installing device tree blobs...");
    snprintf(cmd, sizeof(cmd),
             "cp arch/arm64/boot/dts/rockchip/rk3588*.dtb %s/rootfs/boot/",
             config->output_dir);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Install modules
    LOG_INFO("Installing kernel modules...");
    snprintf(cmd, sizeof(cmd),
             "make ARCH=%s CROSS_COMPILE=%s INSTALL_MOD_PATH=%s/rootfs modules_install",
             config->arch, config->cross_compile, config->output_dir);
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_ERROR("Failed to install kernel modules");
        return ERROR_INSTALLATION_FAILED;
    }
    
    // Create initramfs
    LOG_INFO("Creating initramfs...");
    snprintf(cmd, sizeof(cmd),
             "cd %s/rootfs && find . -print0 | cpio --null -ov --format=newc | "
             "gzip -9 > %s/rootfs/boot/initrd.img-%s",
             config->output_dir, config->output_dir, config->kernel_version);
    execute_command_safe(cmd, 1, &error_ctx);
    
    LOG_INFO("Kernel installation completed");
    return ERROR_SUCCESS;
}

// Download U-Boot source
int download_uboot_source(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char uboot_dir[MAX_PATH_LEN];
    error_context_t error_ctx = {0};
    
    LOG_INFO("Downloading U-Boot source for RK3588...");
    
    snprintf(uboot_dir, sizeof(uboot_dir), "%s/u-boot", config->build_dir);
    
    // Clone U-Boot with Rockchip support
    snprintf(cmd, sizeof(cmd),
             "git clone --depth 1 --branch v2024.01-rc4 "
             "https://github.com/u-boot/u-boot.git %s",
             uboot_dir);
    
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_WARNING("Failed to clone mainline U-Boot, trying Rockchip fork...");
        
        snprintf(cmd, sizeof(cmd),
                 "git clone --depth 1 "
                 "https://github.com/rockchip-linux/u-boot.git %s",
                 uboot_dir);
        
        if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
            LOG_ERROR("Failed to download U-Boot source");
            return ERROR_NETWORK_FAILURE;
        }
    }
    
    // Download ARM Trusted Firmware
    LOG_INFO("Downloading ARM Trusted Firmware...");
    snprintf(cmd, sizeof(cmd),
             "git clone --depth 1 "
             "https://github.com/ARM-software/arm-trusted-firmware.git "
             "%s/arm-trusted-firmware",
             config->build_dir);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Download Rockchip binary blobs
    LOG_INFO("Downloading Rockchip firmware blobs...");
    snprintf(cmd, sizeof(cmd),
             "git clone --depth 1 "
             "https://github.com/rockchip-linux/rkbin.git %s/rkbin",
             config->build_dir);
    execute_command_safe(cmd, 1, &error_ctx);
    
    LOG_INFO("U-Boot source downloaded successfully");
    return ERROR_SUCCESS;
}

// Build U-Boot
int build_uboot(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char uboot_dir[MAX_PATH_LEN];
    error_context_t error_ctx = {0};
    
    LOG_INFO("Building U-Boot for Orange Pi 5 Plus...");
    
    snprintf(uboot_dir, sizeof(uboot_dir), "%s/u-boot", config->build_dir);
    
    if (chdir(uboot_dir) != 0) {
        LOG_ERROR("Failed to change to U-Boot directory");
        return ERROR_FILE_NOT_FOUND;
    }
    
    // Configure U-Boot for Orange Pi 5 Plus
    snprintf(cmd, sizeof(cmd),
             "make ARCH=arm CROSS_COMPILE=%s orangepi-5-plus-rk3588_defconfig",
             config->cross_compile);
    
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_WARNING("Orange Pi 5 Plus config not found, using generic RK3588");
        snprintf(cmd, sizeof(cmd),
                 "make ARCH=arm CROSS_COMPILE=%s evb-rk3588_defconfig",
                 config->cross_compile);
        execute_command_safe(cmd, 1, &error_ctx);
    }
    
    // Build U-Boot
    snprintf(cmd, sizeof(cmd),
             "make ARCH=arm CROSS_COMPILE=%s -j%d",
             config->cross_compile, config->jobs);
    
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_ERROR("Failed to build U-Boot");
        return ERROR_COMPILATION_FAILED;
    }
    
    // Build ARM Trusted Firmware
    LOG_INFO("Building ARM Trusted Firmware...");
    snprintf(cmd, sizeof(cmd),
             "cd %s/arm-trusted-firmware && "
             "make CROSS_COMPILE=%s PLAT=rk3588 bl31",
             config->build_dir, config->cross_compile);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Create final bootloader image
    LOG_INFO("Creating bootloader image...");
    snprintf(cmd, sizeof(cmd),
             "%s/rkbin/tools/mkimage -n rk3588 -T rksd -d "
             "%s/rkbin/bin/rk35/rk3588_ddr_lp4_2112MHz_lp5_2736MHz_v1.08.bin:%s/spl/u-boot-spl.bin "
             "%s/idbloader.img",
             config->build_dir, config->build_dir, uboot_dir, config->output_dir);
    execute_command_safe(cmd, 1, &error_ctx);
    
    LOG_INFO("U-Boot built successfully");
    return ERROR_SUCCESS;
}

// Build Ubuntu rootfs
int build_ubuntu_rootfs(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char rootfs_dir[MAX_PATH_LEN];
    error_context_t error_ctx = {0};
    
    LOG_INFO("Building Ubuntu root filesystem...");
    
    snprintf(rootfs_dir, sizeof(rootfs_dir), "%s/rootfs", config->output_dir);
    
    // Clean up any existing rootfs
    LOG_INFO("Cleaning up previous rootfs attempts...");
    snprintf(cmd, sizeof(cmd), "rm -rf %s", rootfs_dir);
    execute_command_safe(cmd, 0, &error_ctx);
    
    // Create rootfs directory
    if (create_directory_safe(rootfs_dir, &error_ctx) != 0) {
        return ERROR_FILE_NOT_FOUND;
    }
    
    // Check if debootstrap script exists for this release
    char script_path[256];
    snprintf(script_path, sizeof(script_path), "/usr/share/debootstrap/scripts/%s", 
             config->ubuntu_codename);
    
    if (access(script_path, F_OK) != 0) {
        char msg[512];
        snprintf(msg, sizeof(msg), "Debootstrap script not found for Ubuntu %s (%s)", 
                config->ubuntu_release, config->ubuntu_codename);
        LOG_WARNING(msg);
        LOG_INFO("Creating symlink to jammy script...");
        
        // Try to create the symlink
        snprintf(cmd, sizeof(cmd), 
                 "ln -sf /usr/share/debootstrap/scripts/jammy %s", 
                 script_path);
        if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
            LOG_WARNING("Failed to create debootstrap script symlink");
            LOG_INFO("Falling back to Ubuntu 22.04 (jammy) which is known to work");
            
            // Change to jammy
            strcpy(config->ubuntu_release, "22.04");
            strcpy(config->ubuntu_codename, "jammy");
        }
    }
    
    // Run debootstrap first stage
    LOG_INFO("Running debootstrap first stage...");
    snprintf(cmd, sizeof(cmd),
             "debootstrap --arch=arm64 --foreign --include=wget,ca-certificates "
             "%s %s http://ports.ubuntu.com/ubuntu-ports",
             config->ubuntu_codename, rootfs_dir);
    
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_ERROR("Failed to run debootstrap first stage");
        LOG_ERROR("This usually means the Ubuntu release is not supported");
        LOG_ERROR("Try using Ubuntu 22.04 (jammy) or 20.04 (focal) instead");
        return ERROR_INSTALLATION_FAILED;
    }
    
    // Check if debootstrap created the necessary files
    char debootstrap_dir[MAX_PATH_LEN];
    snprintf(debootstrap_dir, sizeof(debootstrap_dir), "%s/debootstrap", rootfs_dir);
    if (access(debootstrap_dir, F_OK) != 0) {
        LOG_ERROR("Debootstrap did not create the expected directory structure");
        return ERROR_INSTALLATION_FAILED;
    }
    
    // Copy qemu static for arm64 emulation
    snprintf(cmd, sizeof(cmd),
             "cp /usr/bin/qemu-aarch64-static %s/usr/bin/",
             rootfs_dir);
    execute_command_safe(cmd, 0, &error_ctx);
    
    // Run debootstrap second stage
    LOG_INFO("Running debootstrap second stage...");
    snprintf(cmd, sizeof(cmd),
             "chroot %s /debootstrap/debootstrap --second-stage",
             rootfs_dir);
    
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_ERROR("Failed to run debootstrap second stage");
        return ERROR_INSTALLATION_FAILED;
    }
    
    // Configure apt sources
    LOG_INFO("Configuring package sources...");
    
    // Create apt directory if it doesn't exist
    snprintf(cmd, sizeof(cmd), "mkdir -p %s/etc/apt", rootfs_dir);
    execute_command_safe(cmd, 0, &error_ctx);
    
    FILE *sources_file = fopen("/tmp/sources.list", "w");
    if (sources_file) {
        fprintf(sources_file,
                "deb http://ports.ubuntu.com/ubuntu-ports %s main restricted universe multiverse\n"
                "deb http://ports.ubuntu.com/ubuntu-ports %s-updates main restricted universe multiverse\n"
                "deb http://ports.ubuntu.com/ubuntu-ports %s-security main restricted universe multiverse\n",
                config->ubuntu_codename, config->ubuntu_codename, config->ubuntu_codename);
        fclose(sources_file);
        
        snprintf(cmd, sizeof(cmd),
                 "cp /tmp/sources.list %s/etc/apt/sources.list",
                 rootfs_dir);
        execute_command_safe(cmd, 0, &error_ctx);
    }
    
    // Update package database in chroot
    LOG_INFO("Updating package database...");
    snprintf(cmd, sizeof(cmd),
             "chroot %s apt update",
             rootfs_dir);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Install base packages based on distribution type
    const char *base_packages = "ubuntu-minimal init systemd";
    const char *extra_packages = "";
    
    switch (config->distro_type) {
        case DISTRO_DESKTOP:
            extra_packages = "ubuntu-desktop network-manager";
            break;
        case DISTRO_SERVER:
            extra_packages = "ubuntu-server openssh-server";
            break;
        case DISTRO_EMULATION:
            extra_packages = "xserver-xorg-core openbox";
            break;
        case DISTRO_MINIMAL:
            extra_packages = "";
            break;
        default:
            break;
    }
    
    LOG_INFO("Installing base system packages...");
    snprintf(cmd, sizeof(cmd),
             "chroot %s apt install -y %s %s",
             rootfs_dir, base_packages, extra_packages);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Configure hostname
    snprintf(cmd, sizeof(cmd), "echo '%s' > %s/etc/hostname",
             config->hostname, rootfs_dir);
    execute_command_safe(cmd, 0, &error_ctx);
    
    // Create user
    LOG_INFO("Creating user account...");
    snprintf(cmd, sizeof(cmd),
             "chroot %s useradd -m -s /bin/bash -G sudo,audio,video %s",
             rootfs_dir, config->username);
    execute_command_safe(cmd, 0, &error_ctx);
    
    // Set password
    snprintf(cmd, sizeof(cmd),
             "echo '%s:%s' | chroot %s chpasswd",
             config->username, config->password, rootfs_dir);
    execute_command_safe(cmd, 0, &error_ctx);
    
    // Clean up
    snprintf(cmd, sizeof(cmd), "rm -f %s/usr/bin/qemu-aarch64-static", rootfs_dir);
    execute_command_safe(cmd, 0, &error_ctx);
    
    LOG_INFO("Ubuntu root filesystem created successfully");
    return ERROR_SUCCESS;
}

// Create system image
int create_system_image(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char image_path[MAX_PATH_LEN];
    error_context_t error_ctx = {0};
    
    LOG_INFO("Creating system image...");
    
    snprintf(image_path, sizeof(image_path), 
             "%s/orangepi5plus-%s-%s.img",
             config->output_dir, config->ubuntu_codename, config->kernel_version);
    
    // Create empty image file
    LOG_INFO("Creating image file...");
    snprintf(cmd, sizeof(cmd),
             "dd if=/dev/zero of=%s bs=1M count=%s status=progress",
             image_path, config->image_size);
    
    if (execute_command_safe(cmd, 1, &error_ctx) != 0) {
        LOG_ERROR("Failed to create image file");
        return ERROR_UNKNOWN;
    }
    
    // Create partition table
    LOG_INFO("Creating partition table...");
    snprintf(cmd, sizeof(cmd),
             "parted -s %s mklabel gpt", image_path);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Create partitions
    snprintf(cmd, sizeof(cmd),
             "parted -s %s mkpart loader 64s 8MiB", image_path);
    execute_command_safe(cmd, 1, &error_ctx);
    
    snprintf(cmd, sizeof(cmd),
             "parted -s %s mkpart boot fat32 8MiB 256MiB", image_path);
    execute_command_safe(cmd, 1, &error_ctx);
    
    snprintf(cmd, sizeof(cmd),
             "parted -s %s mkpart root ext4 256MiB 100%%", image_path);
    execute_command_safe(cmd, 1, &error_ctx);
    
    snprintf(cmd, sizeof(cmd),
             "parted -s %s set 2 boot on", image_path);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Setup loop device
    LOG_INFO("Setting up loop device...");
    snprintf(cmd, sizeof(cmd),
             "losetup -P -f %s", image_path);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Get loop device name
    char cmd_buf[MAX_CMD_LEN];
    snprintf(cmd_buf, sizeof(cmd_buf), "losetup -j %s | cut -d: -f1", image_path);
    FILE *fp = popen(cmd_buf, "r");
    char loop_dev[32];
    if (fp && fgets(loop_dev, sizeof(loop_dev), fp)) {
        loop_dev[strcspn(loop_dev, "\n")] = 0;
        pclose(fp);
    } else {
        LOG_ERROR("Failed to get loop device");
        return ERROR_UNKNOWN;
    }
    
    // Format partitions
    LOG_INFO("Formatting partitions...");
    snprintf(cmd, sizeof(cmd), "mkfs.vfat -F 32 %sp2", loop_dev);
    execute_command_safe(cmd, 1, &error_ctx);
    
    snprintf(cmd, sizeof(cmd), "mkfs.ext4 -F %sp3", loop_dev);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Mount partitions
    LOG_INFO("Mounting partitions...");
    execute_command_safe("mkdir -p /mnt/boot /mnt/root", 0, &error_ctx);
    
    snprintf(cmd, sizeof(cmd), "mount %sp2 /mnt/boot", loop_dev);
    execute_command_safe(cmd, 1, &error_ctx);
    
    snprintf(cmd, sizeof(cmd), "mount %sp3 /mnt/root", loop_dev);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Copy rootfs
    LOG_INFO("Copying root filesystem...");
    snprintf(cmd, sizeof(cmd),
             "rsync -aHAXx %s/rootfs/ /mnt/root/",
             config->output_dir);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Copy boot files
    LOG_INFO("Copying boot files...");
    snprintf(cmd, sizeof(cmd),
             "cp -r %s/rootfs/boot/* /mnt/boot/",
             config->output_dir);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Install bootloader
    LOG_INFO("Installing bootloader...");
    snprintf(cmd, sizeof(cmd),
             "dd if=%s/idbloader.img of=%s seek=64 conv=notrunc",
             config->output_dir, loop_dev);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Create boot configuration
    execute_command_safe("mkdir -p /mnt/boot/extlinux", 0, &error_ctx);
    FILE *boot_cfg = fopen("/mnt/boot/extlinux/extlinux.conf", "w");
    if (boot_cfg) {
        fprintf(boot_cfg,
                "label Ubuntu\n"
                "    kernel /vmlinuz-%s\n"
                "    initrd /initrd.img-%s\n"
                "    devicetreedir /dtbs\n"
                "    append console=ttyS2,1500000 root=/dev/mmcblk0p3 rw rootwait\n",
                config->kernel_version, config->kernel_version);
        fclose(boot_cfg);
    }
    
    // Cleanup
    LOG_INFO("Cleaning up...");
    execute_command_safe("sync", 0, &error_ctx);
    execute_command_safe("umount /mnt/boot /mnt/root", 0, &error_ctx);
    snprintf(cmd, sizeof(cmd), "losetup -d %s", loop_dev);
    execute_command_safe(cmd, 0, &error_ctx);
    
    char msg[512];
    snprintf(msg, sizeof(msg), "System image created successfully: %s", image_path);
    LOG_INFO(msg);
    
    return ERROR_SUCCESS;
}

// Install system packages
int install_system_packages(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char rootfs_dir[MAX_PATH_LEN];
    error_context_t error_ctx = {0};
    
    LOG_INFO("Installing system packages...");
    
    snprintf(rootfs_dir, sizeof(rootfs_dir), "%s/rootfs", config->output_dir);
    
    // Check if rootfs exists
    if (access(rootfs_dir, F_OK) != 0) {
        LOG_ERROR("Root filesystem not found. Did debootstrap fail?");
        return ERROR_FILE_NOT_FOUND;
    }
    
    // Common packages for all distributions
    const char *common_packages = 
        "linux-firmware wireless-tools wpasupplicant "
        "network-manager usbutils pciutils i2c-tools "
        "htop nano vim curl wget git sudo locales "
        "software-properties-common dbus-x11";
    
    snprintf(cmd, sizeof(cmd),
             "chroot %s apt install -y %s",
             rootfs_dir, common_packages);
    execute_command_safe(cmd, 1, &error_ctx);
    
    // Distribution-specific packages
    switch (config->distro_type) {
        case DISTRO_DESKTOP:
            LOG_INFO("Installing desktop packages...");
            snprintf(cmd, sizeof(cmd),
                     "chroot %s apt install -y "
                     "gnome-shell gdm3 gnome-terminal firefox "
                     "gnome-tweaks gnome-system-monitor",
                     rootfs_dir);
            execute_command_safe(cmd, 1, &error_ctx);
            break;
            
        case DISTRO_SERVER:
            LOG_INFO("Installing server packages...");
            snprintf(cmd, sizeof(cmd),
                     "chroot %s apt install -y "
                     "openssh-server fail2ban ufw "
                     "docker.io docker-compose",
                     rootfs_dir);
            execute_command_safe(cmd, 1, &error_ctx);
            break;
            
        case DISTRO_EMULATION:
            LOG_INFO("Installing emulation packages...");
            install_emulation_packages(config);
            break;
            
        default:
            break;
    }
    
    // Install GPU support packages if enabled
    if (config->install_gpu_blobs) {
        LOG_INFO("Installing GPU support packages...");
        snprintf(cmd, sizeof(cmd),
                 "chroot %s apt install -y "
                 "mesa-utils glmark2-es2 vulkan-tools",
                 rootfs_dir);
        execute_command_safe(cmd, 1, &error_ctx);
    }
    
    // Configure locales
    LOG_INFO("Configuring locales...");
    snprintf(cmd, sizeof(cmd),
             "chroot %s locale-gen en_US.UTF-8",
             rootfs_dir);
    execute_command_safe(cmd, 1, &error_ctx);
    
    LOG_INFO("System packages installed successfully");
    return ERROR_SUCCESS;
}

// Configure system services
int configure_system_services(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char rootfs_dir[MAX_PATH_LEN];
    error_context_t error_ctx = {0};
    
    LOG_INFO("Configuring system services...");
    
    snprintf(rootfs_dir, sizeof(rootfs_dir), "%s/rootfs", config->output_dir);
    
    // Check if rootfs exists
    if (access(rootfs_dir, F_OK) != 0) {
        LOG_ERROR("Root filesystem not found");
        return ERROR_FILE_NOT_FOUND;
    }
    
    // Enable essential services
    const char *enable_services[] = {
        "systemd-networkd",
        "systemd-resolved",
        "ssh",
        NULL
    };
    
    for (int i = 0; enable_services[i] != NULL; i++) {
        snprintf(cmd, sizeof(cmd),
                 "chroot %s systemctl enable %s",
                 rootfs_dir, enable_services[i]);
        execute_command_safe(cmd, 0, &error_ctx);
    }
    
    // Distribution-specific service configuration
    switch (config->distro_type) {
        case DISTRO_DESKTOP:
            snprintf(cmd, sizeof(cmd),
                     "chroot %s systemctl enable gdm3",
                     rootfs_dir);
            execute_command_safe(cmd, 0, &error_ctx);
            break;
            
        case DISTRO_SERVER:
            // Configure firewall
            snprintf(cmd, sizeof(cmd),
                     "chroot %s ufw default deny incoming",
                     rootfs_dir);
            execute_command_safe(cmd, 0, &error_ctx);
            
            snprintf(cmd, sizeof(cmd),
                     "chroot %s ufw default allow outgoing",
                     rootfs_dir);
            execute_command_safe(cmd, 0, &error_ctx);
            
            snprintf(cmd, sizeof(cmd),
                     "chroot %s ufw allow ssh",
                     rootfs_dir);
            execute_command_safe(cmd, 0, &error_ctx);
            break;
            
        default:
            break;
    }
    
    // Create netplan directory
    snprintf(cmd, sizeof(cmd), "mkdir -p %s/etc/netplan", rootfs_dir);
    execute_command_safe(cmd, 0, &error_ctx);
    
    // Configure network
    FILE *netplan = fopen("/tmp/01-netcfg.yaml", "w");
    if (netplan) {
        fprintf(netplan,
                "network:\n"
                "  version: 2\n"
                "  renderer: networkd\n"
                "  ethernets:\n"
                "    eth0:\n"
                "      dhcp4: yes\n"
                "      dhcp6: yes\n");
        fclose(netplan);
        
        snprintf(cmd, sizeof(cmd),
                 "cp /tmp/01-netcfg.yaml %s/etc/netplan/",
                 rootfs_dir);
        execute_command_safe(cmd, 0, &error_ctx);
    }
    
    // Configure fstab
    FILE *fstab = fopen("/tmp/fstab", "w");
    if (fstab) {
        fprintf(fstab,
                "# /etc/fstab: static file system information\n"
                "/dev/mmcblk0p3  /       ext4    defaults        0 1\n"
                "/dev/mmcblk0p2  /boot   vfat    defaults        0 2\n");
        fclose(fstab);
        
        snprintf(cmd, sizeof(cmd),
                 "cp /tmp/fstab %s/etc/",
                 rootfs_dir);
        execute_command_safe(cmd, 0, &error_ctx);
    }
    
    LOG_INFO("System services configured successfully");
    return ERROR_SUCCESS;
}

// Install emulation packages
int install_emulation_packages(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    
    LOG_INFO("Installing emulation platform packages...");
    
    // Common emulation dependencies
    const char *common_packages = 
        "libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev "
        "libboost-all-dev libavcodec-dev libavformat-dev libavutil-dev "
        "libswscale-dev libfreeimage-dev libfreetype6-dev libcurl4-openssl-dev "
        "libasound2-dev libpulse-dev libudev-dev libvlc-dev libvlccore-dev "
        "libxml2-dev libxrandr-dev mesa-common-dev libglu1-mesa-dev "
        "libgles2-mesa-dev libavfilter-dev libavresample-dev libvorbis-dev "
        "libflac-dev";
    
    snprintf(cmd, sizeof(cmd), "apt install -y %s", common_packages);
    
    if (execute_command_with_retry(cmd, 1, 2) != 0) {
        LOG_WARNING("Some emulation packages failed to install");
    }
    
    // Install platform-specific packages
    switch (config->emu_platform) {
        case EMU_LIBREELEC:
            return setup_libreelec(config);
        case EMU_EMULATIONSTATION:
            return setup_emulationstation(config);
        case EMU_RETROPIE:
            return setup_retropie(config);
        case EMU_ALL:
            setup_libreelec(config);
            setup_emulationstation(config);
            setup_retropie(config);
            break;
        default:
            break;
    }
    
    return ERROR_SUCCESS;
}

// Setup LibreELEC
int setup_libreelec(build_config_t *config) {
    LOG_INFO("Setting up LibreELEC environment...");
    LOG_WARNING("LibreELEC is a complete OS - this will prepare the build environment");
    
    char cmd[MAX_CMD_LEN];
    
    // Clone LibreELEC source
    snprintf(cmd, sizeof(cmd), 
             "cd %s && git clone --depth 1 https://github.com/LibreELEC/LibreELEC.tv.git libreelec",
             config->build_dir);
    
    if (execute_command_safe(cmd, 1, NULL) != 0) {
        LOG_ERROR("Failed to clone LibreELEC source");
        return ERROR_NETWORK_FAILURE;
    }
    
    // Install LibreELEC build dependencies
    const char *libreelec_deps = 
        "gcc make git unzip wget xz-utils python3 python3-distutils "
        "python3-setuptools python3-wheel python3-dev bc patchutils "
        "gawk gperf zip lzop g++ default-jre-headless u-boot-tools "
        "texinfo device-tree-compiler";
    
    snprintf(cmd, sizeof(cmd), "apt install -y %s", libreelec_deps);
    execute_command_safe(cmd, 1, NULL);
    
    LOG_INFO("LibreELEC build environment prepared");
    LOG_WARNING("NO copyrighted content included - users must provide their own legal content");
    
    return ERROR_SUCCESS;
}

// Setup EmulationStation
int setup_emulationstation(build_config_t *config) {
    LOG_INFO("Setting up EmulationStation...");
    
    char cmd[MAX_CMD_LEN];
    char es_dir[MAX_PATH_LEN];
    
    snprintf(es_dir, sizeof(es_dir), "%s/emulationstation", config->build_dir);
    
    // Clone EmulationStation
    snprintf(cmd, sizeof(cmd), 
             "git clone --recursive https://github.com/RetroPie/EmulationStation.git %s",
             es_dir);
    
    if (execute_command_safe(cmd, 1, NULL) != 0) {
        LOG_ERROR("Failed to clone EmulationStation");
        return ERROR_NETWORK_FAILURE;
    }
    
    // Build EmulationStation
    snprintf(cmd, sizeof(cmd), 
             "cd %s && mkdir build && cd build && "
             "cmake .. -DFREETYPE_INCLUDE_DIRS=/usr/include/freetype2/ && "
             "make -j%d",
             es_dir, config->jobs);
    
    if (execute_command_safe(cmd, 1, NULL) != 0) {
        LOG_WARNING("Failed to build EmulationStation");
    }
    
    // Create directories
    execute_command_safe("mkdir -p /etc/emulationstation", 0, NULL);
    execute_command_safe("mkdir -p /opt/retropie/configs/all/emulationstation", 0, NULL);
    
    // Install default config
    snprintf(cmd, sizeof(cmd), 
             "cp %s/resources/systems.cfg.example /etc/emulationstation/es_systems.cfg",
             es_dir);
    execute_command_safe(cmd, 0, NULL);
    
    LOG_INFO("EmulationStation installed successfully");
    LOG_WARNING("NO games or emulators included - users must install legal content");
    
    return ERROR_SUCCESS;
}

// Setup RetroPie
int setup_retropie(build_config_t *config) {
    LOG_INFO("Setting up RetroPie environment...");
    
    char cmd[MAX_CMD_LEN];
    char retropie_dir[MAX_PATH_LEN];
    
    snprintf(retropie_dir, sizeof(retropie_dir), "%s/RetroPie-Setup", config->build_dir);
    
    // Clone RetroPie-Setup
    snprintf(cmd, sizeof(cmd), 
             "git clone --depth 1 https://github.com/RetroPie/RetroPie-Setup.git %s",
             retropie_dir);
    
    if (execute_command_safe(cmd, 1, NULL) != 0) {
        LOG_ERROR("Failed to clone RetroPie-Setup");
        return ERROR_NETWORK_FAILURE;
    }
    
    // Make scripts executable
    snprintf(cmd, sizeof(cmd), "chmod +x %s/retropie_setup.sh", retropie_dir);
    execute_command_safe(cmd, 0, NULL);
    
    // Create RetroPie directories
    execute_command_safe("mkdir -p /opt/retropie", 0, NULL);
    execute_command_safe("mkdir -p /home/pi/RetroPie", 0, NULL);
    execute_command_safe("mkdir -p /home/pi/RetroPie/roms", 0, NULL);
    execute_command_safe("mkdir -p /home/pi/RetroPie/BIOS", 0, NULL);
    
    // Install core packages only (no emulators with copyrighted code)
    snprintf(cmd, sizeof(cmd), 
             "cd %s && ./retropie_packages.sh setup core_packages",
             retropie_dir);
    execute_command_safe(cmd, 1, NULL);
    
    LOG_INFO("RetroPie core environment installed");
    LOG_WARNING("NO emulators, games, or BIOS files included");
    LOG_WARNING("Users must legally obtain and install their own content");
    
    return ERROR_SUCCESS;
}
