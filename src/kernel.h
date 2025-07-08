#ifndef KERNEL_H
#define KERNEL_H

#include "config.h"

// Orange Pi 5 Plus kernel functions
int download_kernel_source(void);
int configure_kernel(void);
int build_kernel(void);
int install_kernel(const char *install_path);

// Additional kernel functions
int apply_rk3588_patches(void);
int enable_mali_gpu_support(void);
int enable_orangepi_features(void);

#endif // KERNEL_H
