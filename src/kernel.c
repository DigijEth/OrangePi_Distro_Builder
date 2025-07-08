#include "kernel.h"
#include "logging.h"
#include "system_utils.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Externally defined global configuration
extern build_config_t g_build_config;

// Download kernel source based on global config
int download_kernel_source(void) {
    log_info("Downloading kernel source...");

    char command[1024];
    char kernel_dir[256];
    snprintf(kernel_dir, sizeof(kernel_dir), "%s/kernel", BUILD_DIR);

    // Create build directory and clean existing source
    execute_command("mkdir -p " BUILD_DIR, 1);
    snprintf(command, sizeof(command), "rm -rf %s", kernel_dir);
    execute_command(command, 1);

    log_info("Cloning kernel from %s (branch: %s)", g_build_config.kernel_git_url, g_build_config.kernel_branch);
    snprintf(command, sizeof(command),
        "git clone --depth 1 --branch %s %s %s",
        g_build_config.kernel_branch, g_build_config.kernel_git_url, kernel_dir);

    if (execute_command(command, 1) != 0) {
        log_error("download_kernel_source", "Failed to clone kernel from specified source", 1);
        return -1;
    }

    log_info("Kernel source downloaded successfully to %s", kernel_dir);
    return 0;
}

// Configure the kernel for Orange Pi 5 Plus
int configure_kernel(void) {
    log_info("Configuring kernel...");

    char command[1024];
    char kernel_dir[256];
    snprintf(kernel_dir, sizeof(kernel_dir), "%s/kernel", BUILD_DIR);

    // Run defconfig
    snprintf(command, sizeof(command),
        "cd %s && make ARCH=%s CROSS_COMPILE=%s %s",
        kernel_dir, TARGET_ARCH, CROSS_COMPILE, KERNEL_DEFCONFIG);
    if (execute_command(command, 1) != 0) {
        log_error("configure_kernel", "Kernel 'defconfig' failed", 1);
        return -1;
    }

    // Enable specific features for Orange Pi 5 Plus and gaming
    log_info("Enabling Orange Pi 5 Plus specific features...");
    snprintf(command, sizeof(command),
        "cd %s && "
        "scripts/config --enable CONFIG_PREEMPT_VOLUNTARY && "
        "scripts/config --enable CONFIG_HIGH_RES_TIMERS && "
        "scripts/config --enable CONFIG_SCHED_AUTOGROUP && "
        "scripts/config --enable CONFIG_CFS_BANDWIDTH && "
        "scripts/config --enable CONFIG_RT_GROUP_SCHED && "
        "scripts/config --set-str CONFIG_DEFAULT_CPU_GOV_SCHEDUTIL && "
        "scripts/config --enable CONFIG_ARM_RK3588_CPUFREQ && "
        "scripts/config --enable CONFIG_DRM_PANFROST && "
        "scripts/config --enable CONFIG_DRM_ROCKCHIP",
        kernel_dir);
    execute_command(command, 1);

    log_info("Kernel configured successfully.");
    return 0;
}

// Build the kernel
int build_kernel(void) {
    log_info("Building the kernel...");

    char command[1024];
    char kernel_dir[256];
    int cpu_cores = get_cpu_cores();
    snprintf(kernel_dir, sizeof(kernel_dir), "%s/kernel", BUILD_DIR);

    log_info("Compiling kernel with %d cores...", cpu_cores);
    snprintf(command, sizeof(command),
        "cd %s && "
        "make ARCH=%s CROSS_COMPILE=%s -j%d Image modules dtbs",
        kernel_dir, TARGET_ARCH, CROSS_COMPILE, cpu_cores);

    if (execute_command(command, 1) != 0) {
        log_error("build_kernel", "Kernel compilation failed", 1);
        return -1;
    }

    log_info("Kernel build completed successfully.");
    return 0;
}

// Install the kernel to the specified rootfs path
int install_kernel(const char *install_path) {
    log_info("Installing kernel to rootfs at %s...", install_path);

    char command[1024];
    char kernel_dir[256];
    snprintf(kernel_dir, sizeof(kernel_dir), "%s/kernel", BUILD_DIR);

    // Create directories in rootfs
    snprintf(command, sizeof(command), "mkdir -p %s/boot %s/lib/modules", install_path, install_path);
    execute_command(command, 1);

    // Install kernel image
    snprintf(command, sizeof(command),
        "cp %s/arch/%s/boot/Image %s/boot/",
        kernel_dir, TARGET_ARCH, install_path);
    execute_command(command, 1);

    // Install device tree blobs
    snprintf(command, sizeof(command),
        "cp %s/arch/%s/boot/dts/rockchip/rk3588-orangepi-5-plus.dtb %s/boot/ 2>/dev/null || "
        "cp %s/arch/%s/boot/dts/rockchip/rk3588*.dtb %s/boot/",
        kernel_dir, TARGET_ARCH, install_path,
        kernel_dir, TARGET_ARCH, install_path);
    execute_command(command, 1);

    // Install kernel modules
    snprintf(command, sizeof(command),
        "cd %s && "
        "make ARCH=%s CROSS_COMPILE=%s INSTALL_MOD_PATH=%s modules_install",
        kernel_dir, TARGET_ARCH, CROSS_COMPILE, install_path);
    execute_command(command, 1);

    log_info("Kernel installation completed.");
    return 0;
}
