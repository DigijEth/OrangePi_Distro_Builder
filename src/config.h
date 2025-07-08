#ifndef CONFIG_H
#define CONFIG_H

#define VERSION "2.0.0"
#define BUILD_DIR "/tmp/orangepi_build"
#define LOG_FILE "/tmp/orangepi_build.log"
#define MAX_CMD_LEN 2048
#define MAX_PATH_LEN 512

// Orange Pi 5 Plus specific configuration
#define KERNEL_SOURCE_DIR "/tmp/orangepi_build/kernel"
#define UBOOT_SOURCE_DIR "/tmp/orangepi_build/uboot"
#define ROOTFS_PATH "/tmp/orangepi_build/rootfs"
#define OUTPUT_DIR "/tmp/orangepi_build/output"
#define PATCHES_DIR "/tmp/orangepi_build/patches"

// Orange Pi 5 Plus hardware configuration
#define TARGET_ARCH "arm64"
#define CROSS_COMPILE "aarch64-linux-gnu-"
#define SOC_FAMILY "rk3588"
#define BOARD_NAME "orangepi-5-plus"

// Source repositories (Orange Pi official sources)
#define KERNEL_REPO_URL "https://github.com/orangepi-xunlong/linux-orangepi.git"
#define KERNEL_BRANCH "orange-pi-5.10-rk35xx"
#define KERNEL_DEFCONFIG "rockchip_linux_defconfig"

#define UBOOT_REPO_URL "https://github.com/orangepi-xunlong/u-boot-orangepi.git"
#define UBOOT_BRANCH "v2017.09-rk3588"
#define UBOOT_DEFCONFIG "orangepi_5_plus_defconfig"

// Default sources, can be overridden by user selection
#define KERNEL_GIT_URL_DEFAULT KERNEL_REPO_URL
#define KERNEL_BRANCH_DEFAULT KERNEL_BRANCH
#define UBOOT_GIT_URL_DEFAULT UBOOT_REPO_URL
#define UBOOT_BRANCH_DEFAULT UBOOT_BRANCH

// Mali GPU driver sources
#define MALI_DRIVER_URL "https://developer.arm.com/tools-and-software/graphics-and-gaming/mali-drivers/kernel"
#define PANFROST_MESA_URL "https://gitlab.freedesktop.org/mesa/mesa.git"

// Ubuntu base configuration  
#define UBUNTU_VERSION "22.04"
#define UBUNTU_CODENAME "jammy"
#define UBUNTU_MIRROR "http://ports.ubuntu.com/"
#define UBUNTU_ARCH "arm64"

// Color codes for output
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

// Build configuration structure
typedef struct {
    char kernel_version[64];
    char build_dir[MAX_PATH_LEN];
    char cross_compile[128];
    char arch[16];
    char defconfig[64];
    int jobs;
    int verbose;
    int clean_build;
    int install_gpu_drivers;
    int enable_opencl;
    int enable_vulkan;
    int build_rootfs;
    int build_uboot;
    int create_image;
    int gaming_build;
    int include_emulators;
    char output_dir[MAX_PATH_LEN];
    char image_size[32];
    // Custom source repositories
    char kernel_git_url[MAX_PATH_LEN];
    char kernel_branch[128];
    char uboot_git_url[MAX_PATH_LEN];
    char uboot_branch[128];
    // API credentials
    char github_token[256];
    char gitlab_token[256];
    char arm_user[128];
    char arm_password[256];
} build_config_t;

// Global build configuration instance, defined in builder.c
extern build_config_t g_build_config;

#endif // CONFIG_H
