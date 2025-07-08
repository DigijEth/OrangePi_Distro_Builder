#ifndef IMAGE_H
#define IMAGE_H

#include "config.h"

// Main function to create the final bootable image
int create_boot_image(const char* config_path);

// Orange Pi specific image functions
int create_image_file(const char* image_path, long long image_size_mb);
int partition_orangepi_image(const char* image_path);
int format_orangepi_partitions(const char* image_path);
int mount_orangepi_partitions(const char* image_path, const char* mount_point);
int copy_rootfs_to_image(const char* rootfs_path, const char* mount_point);
int install_bootloader_to_image(const char* image_path);
int configure_boot_files(const char* mount_point);
int unmount_orangepi_partitions(const char* mount_point);
int compress_final_image(const char* image_path);

// Legacy functions for compatibility
int create_system_image(build_config_t *config);
int partition_image(const char* image_path);
int format_partitions(const char* image_path);
int mount_partitions(const char* image_path, const char* mount_point);
int unmount_partitions(const char* mount_point);

#endif // IMAGE_H
