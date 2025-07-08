#ifndef ROOTFS_H
#define ROOTFS_H

#include "config.h"

// New Orange Pi specific rootfs functions
int build_rootfs(const char* rootfs_path);
int configure_orangepi_rootfs(const char* rootfs_path);
int install_orangepi_packages(const char* rootfs_path);
int configure_gpu_drivers(const char* rootfs_path);

// Legacy functions for compatibility
int build_ubuntu_rootfs(build_config_t *config);
int install_system_packages(build_config_t *config);
int configure_system_services(build_config_t *config);

#endif // ROOTFS_H
