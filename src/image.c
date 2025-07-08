#include "image.h"
#include "logging.h"
#include "system_utils.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Create bootable Orange Pi 5 Plus image
int create_boot_image(const char* config_path) {
    (void)config_path; // Suppress unused parameter warning
    log_info("Creating bootable Orange Pi 5 Plus Ubuntu image...");

    char image_path[512];
    char command[2048];
    long long image_size_mb = 6144; // 6GB for full system
    const char* mount_point = "/mnt/orangepi_image";
    
    // Generate timestamped image name
    snprintf(image_path, sizeof(image_path), 
        "%s/orangepi-5-plus-ubuntu-%s-$(date +%%Y%%m%%d).img", 
        OUTPUT_DIR, UBUNTU_VERSION);

    // Create output directory
    snprintf(command, sizeof(command), "mkdir -p %s", OUTPUT_DIR);
    execute_command(command, 1);

    // Create blank image file
    if (create_image_file(image_path, image_size_mb) != 0) {
        log_error("create_boot_image", "Failed to create image file", 1);
        return -1;
    }

    // Partition the image for Orange Pi 5 Plus
    if (partition_orangepi_image(image_path) != 0) {
        log_error("create_boot_image", "Failed to partition image", 1);
        return -1;
    }

    // Format partitions
    if (format_orangepi_partitions(image_path) != 0) {
        log_error("create_boot_image", "Failed to format partitions", 1);
        return -1;
    }

    // Mount partitions
    if (mount_orangepi_partitions(image_path, mount_point) != 0) {
        log_error("create_boot_image", "Failed to mount partitions", 1);
        return -1;
    }

    // Copy rootfs to image
    if (copy_rootfs_to_image(ROOTFS_PATH, mount_point) != 0) {
        log_error("create_boot_image", "Failed to copy rootfs", 1);
        unmount_orangepi_partitions(mount_point);
        return -1;
    }

    // Install bootloader to image
    if (install_bootloader_to_image(image_path) != 0) {
        log_error("create_boot_image", "Failed to install bootloader", 1);
        unmount_orangepi_partitions(mount_point);
        return -1;
    }

    // Configure boot files
    if (configure_boot_files(mount_point) != 0) {
        log_error("create_boot_image", "Failed to configure boot files", 1);
        unmount_orangepi_partitions(mount_point);
        return -1;
    }

    // Unmount and finalize
    unmount_orangepi_partitions(mount_point);

    // Compress image
    compress_final_image(image_path);

    log_info("Bootable Orange Pi 5 Plus image created successfully: %s", image_path);
    return 0;
}

// Create blank image file
int create_image_file(const char* image_path, long long size_mb) {
    log_info("Creating %lld MB image file: %s", size_mb, image_path);
    
    char command[1024];
    snprintf(command, sizeof(command), 
        "dd if=/dev/zero of=%s bs=1M count=%lld status=progress",
        image_path, size_mb);
    
    return execute_command(command, 1);
}

// Partition image for Orange Pi 5 Plus (GPT layout)
int partition_orangepi_image(const char* image_path) {
    log_info("Creating Orange Pi 5 Plus partition layout...");
    
    char command[2048];
    
    // Create GPT partition table optimized for Orange Pi 5 Plus
    snprintf(command, sizeof(command),
        "sgdisk --zap-all %s && "
        "sgdisk --clear "
        "--new=1:64:8191 --change-name=1:'loader1' --typecode=1:8301 "
        "--new=2:8192:16383 --change-name=2:'loader2' --typecode=2:8301 "
        "--new=3:16384:24575 --change-name=3:'trust' --typecode=3:8301 "
        "--new=4:24576:32767 --change-name=4:'boot' --typecode=4:8300 "
        "--new=5:32768:-1 --change-name=5:'rootfs' --typecode=5:8300 "
        "%s", image_path, image_path);
    
    return execute_command(command, 1);
}

// Format Orange Pi partitions
int format_orangepi_partitions(const char* image_path) {
    log_info("Formatting Orange Pi partitions...");
    
    char command[1024];
    
    // Setup loop device
    snprintf(command, sizeof(command), 
        "LOOP_DEV=$(losetup --find --show --partscan %s) && "
        "mkfs.fat -F 32 -n BOOT ${LOOP_DEV}p4 && "
        "mkfs.ext4 -L ROOTFS ${LOOP_DEV}p5 && "
        "losetup -d $LOOP_DEV", image_path);
    
    return execute_command(command, 1);
}

// Mount Orange Pi partitions
int mount_orangepi_partitions(const char* image_path, const char* mount_point) {
    log_info("Mounting Orange Pi partitions...");
    
    char command[2048];
    
    // Create mount points and mount partitions
    snprintf(command, sizeof(command),
        "mkdir -p %s %s/boot && "
        "LOOP_DEV=$(losetup --find --show --partscan %s) && "
        "mount ${LOOP_DEV}p5 %s && "
        "mount ${LOOP_DEV}p4 %s/boot && "
        "echo $LOOP_DEV > /tmp/orangepi_loop_device",
        mount_point, mount_point, image_path, mount_point, mount_point);
    
    return execute_command(command, 1);
}

// Copy rootfs to mounted image
int copy_rootfs_to_image(const char* rootfs_path, const char* mount_point) {
    log_info("Copying rootfs to image...");
    
    char command[1024];
    
    // Copy all rootfs contents except /boot
    snprintf(command, sizeof(command),
        "rsync -av --exclude=/boot/* %s/ %s/ && "
        "mkdir -p %s/boot", 
        rootfs_path, mount_point, mount_point);
    
    if (execute_command(command, 1) != 0) {
        return -1;
    }
    
    // Copy kernel and device tree to boot partition
    snprintf(command, sizeof(command),
        "cp %s/boot/* %s/boot/ 2>/dev/null || true",
        rootfs_path, mount_point);
    execute_command(command, 1);
    
    return 0;
}

// Install U-Boot bootloader to image
int install_bootloader_to_image(const char* image_path) {
    log_info("Installing U-Boot bootloader to image...");
    
    char command[1024];
    
    // Flash U-Boot to the appropriate sectors
    if (access(UBOOT_SOURCE_DIR "/uboot/u-boot-rockchip.bin", F_OK) == 0) {
        snprintf(command, sizeof(command),
            "dd if=%s/uboot/u-boot-rockchip.bin of=%s seek=64 conv=notrunc,fsync",
            BUILD_DIR, image_path);
    } else {
        // Fallback method with separate files
        snprintf(command, sizeof(command),
            "dd if=%s/uboot/idbloader.img of=%s seek=64 conv=notrunc,fsync && "
            "dd if=%s/uboot/u-boot.itb of=%s seek=16384 conv=notrunc,fsync",
            BUILD_DIR, image_path, BUILD_DIR, image_path);
    }
    
    return execute_command(command, 1);
}

// Configure boot files for Orange Pi 5 Plus
int configure_boot_files(const char* mount_point) {
    log_info("Configuring boot files...");
    
    char command[2048];
    
    // Create boot.cmd for U-Boot
    snprintf(command, sizeof(command),
        "cat > %s/boot/boot.cmd << 'EOF'\n"
        "# Orange Pi 5 Plus Boot Script\n"
        "setenv bootargs \"root=LABEL=ROOTFS rootwait rw console=ttyS2,1500000 console=tty1 consoleblank=0 loglevel=1 ubootpart=\\${partition} usb-storage.quirks=\\${usbstoragequirks} \\${extraargs}\"\n"
        "if load mmc \\${devnum}:1 \\${kernel_addr_r} /Image; then\n"
        "  if load mmc \\${devnum}:1 \\${fdt_addr_r} /rk3588-orangepi-5-plus.dtb; then\n"
        "    if load mmc \\${devnum}:1 \\${ramdisk_addr_r} /initrd.img; then\n"
        "      booti \\${kernel_addr_r} \\${ramdisk_addr_r}:\\${filesize} \\${fdt_addr_r};\n"
        "    else\n"
        "      booti \\${kernel_addr_r} - \\${fdt_addr_r};\n"
        "    fi;\n"
        "  fi;\n"
        "fi;\n"
        "EOF", mount_point);
    execute_command(command, 1);
    
    // Compile boot.cmd to boot.scr
    snprintf(command, sizeof(command),
        "mkimage -C none -A arm64 -T script -d %s/boot/boot.cmd %s/boot/boot.scr",
        mount_point, mount_point);
    execute_command(command, 1);
    
    // Create armbianEnv.txt for additional configuration
    snprintf(command, sizeof(command),
        "cat > %s/boot/armbianEnv.txt << 'EOF'\n"
        "verbosity=1\n"
        "bootlogo=false\n"
        "console=both\n"
        "disp_mode=1920x1080p60\n"
        "overlay_prefix=rockchip\n"
        "rootdev=UUID=%s\n"
        "rootfstype=ext4\n"
        "usbstoragequirks=0x2537:0x1066:u,0x2537:0x1068:u\n"
        "EOF", mount_point, "$(blkid -s UUID -o value /dev/disk/by-label/ROOTFS)");
    execute_command(command, 1);
    
    return 0;
}

// Unmount Orange Pi partitions and cleanup
int unmount_orangepi_partitions(const char* mount_point) {
    log_info("Unmounting partitions...");
    
    char command[1024];
    
    snprintf(command, sizeof(command),
        "umount %s/boot 2>/dev/null || true && "
        "umount %s 2>/dev/null || true && "
        "if [ -f /tmp/orangepi_loop_device ]; then "
        "  losetup -d $(cat /tmp/orangepi_loop_device) 2>/dev/null || true; "
        "  rm -f /tmp/orangepi_loop_device; "
        "fi", mount_point, mount_point);
    
    return execute_command(command, 1);
}

// Compress final image
int compress_final_image(const char* image_path) {
    log_info("Compressing final image...");
    
    char command[1024];
    
    snprintf(command, sizeof(command),
        "cd $(dirname %s) && "
        "xz -9 -T 0 $(basename %s) && "
        "sha256sum $(basename %s).xz > $(basename %s).xz.sha256",
        image_path, image_path, image_path, image_path);
    
    execute_command(command, 1);
    
    return 0;
}

// Legacy wrapper functions
int create_system_image(build_config_t *config) {
    (void)config; // Suppress unused parameter warning
    return create_boot_image(NULL);
}

int partition_image(const char* image_path) {
    return partition_orangepi_image(image_path);
}

int format_partitions(const char* image_path) {
    return format_orangepi_partitions(image_path);
}

int mount_partitions(const char* image_path, const char* mount_point) {
    return mount_orangepi_partitions(image_path, mount_point);
}

int unmount_partitions(const char* mount_point) {
    return unmount_orangepi_partitions(mount_point);
}

void image_creation_menu(void) {
    char choice[10];
    int choice_int;

    while (1) {
        printf("\n%s%s--- Image Creation Menu ---%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
        printf("1. Create Boot Image\n");
        printf("2. Create System Image\n");
        printf("3. Create Complete Image\n");
        printf("4. Return to Main Menu\n");
        printf("Enter your choice: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) continue;
        choice_int = atoi(choice);

        switch (choice_int) {
            case 1:
                create_boot_image(NULL);
                break;
            case 2:
                create_system_image(&g_build_config);
                break;
            case 3:
                log_info("Complete image creation not yet implemented");
                break;
            case 4:
                return;
            default:
                log_warn("Invalid choice. Please try again.");
        }
    }
}
