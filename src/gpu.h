#ifndef GPU_H
#define GPU_H

#include "config.h"

// This function will orchestrate the download, build, and installation of GPU drivers.
int setup_gpu_drivers(build_config_t *config);

// Downloads the source code for various open-source Mali drivers.
int download_gpu_driver_sources(build_config_t *config);

// Builds the Mesa drivers (like Panfrost) from source.
int build_mesa_drivers(build_config_t *config);

// Installs the compiled drivers and necessary firmware to the target rootfs.
int install_gpu_drivers(build_config_t *config);

// Legacy function for compatibility - installs GPU drivers using a rootfs path
int install_gpu_drivers_legacy(const char* rootfs_path);

// Verifies the GPU driver installation.
int verify_gpu_installation(void);

// Function to install the complete open-source GPU driver stack
int install_mesa_panfrost(const char* rootfs_path, const char* build_dir);
int install_libmali_rockchip(const char* rootfs_path, const char* build_dir);
int install_gpu_firmware(const char* rootfs_path);
void configure_vulkan(const char* rootfs_path);

#endif // GPU_H
