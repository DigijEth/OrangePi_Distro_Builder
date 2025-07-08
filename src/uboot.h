#ifndef UBOOT_H
#define UBOOT_H

#include "config.h"

// Main Orange Pi U-Boot functions
int build_and_install_uboot(const char* config_path);
int download_uboot_source(void);
int apply_orangepi_uboot_patches(void);

// Helper functions for the U-Boot process
int clone_uboot_repo(const char* repo_url, const char* branch, const char* dest_dir);
int apply_uboot_patches(const char* uboot_dir, const char* patches_dir);
int configure_uboot(const char* uboot_dir, const char* defconfig);
int build_uboot(const char* uboot_dir, int num_cores);
int install_uboot(const char* uboot_dir, const char* install_path);

#endif // UBOOT_H
