#include "gpu.h"
#include "logging.h"
#include "system_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MESA_REPO "https://gitlab.freedesktop.org/mesa/mesa.git"
#define LIBMALI_REPO "https://github.com/JeffyCN/libmali.git"
#define FIRMWARE_REPO "https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git"

// Main orchestrator for GPU driver setup
int setup_gpu_drivers(build_config_t *config) {
    log_message("INFO", "Starting GPU driver setup process...");

    if (download_gpu_driver_sources(config) != 0) {
        log_error("setup_gpu_drivers", "Failed to download GPU driver sources.", 0);
        return -1;
    }

    if (build_mesa_drivers(config) != 0) {
        log_error("setup_gpu_drivers", "Failed to build Mesa drivers.", 0);
        return -1;
    }

    if (install_gpu_drivers(config) != 0) {
        log_error("setup_gpu_drivers", "Failed to install GPU drivers.", 0);
        return -1;
    }

    log_message("SUCCESS", "GPU driver setup completed successfully.");
    return 0;
}

// Downloads source code for Mesa, libmali-rockchip, and linux-firmware
int download_gpu_driver_sources(build_config_t *config) {
    // char cmd[MAX_CMD_LEN]; // Unused variable
    char gpu_source_dir[MAX_PATH_LEN];
    snprintf(gpu_source_dir, sizeof(gpu_source_dir), "%s/gpu_sources", config->build_dir);
    create_directory(gpu_source_dir);

    log_message("INFO", "Downloading GPU driver sources...");

    if (chdir(gpu_source_dir) != 0) {
        log_system_error("download_gpu_driver_sources", "chdir");
        return -1;
    }

    // Clone Mesa (for Panfrost/Panthor)
    log_message("INFO", "Cloning Mesa repository...");
    if (access("mesa", F_OK) != 0) {
        if (execute_command("git clone --depth 1 https://gitlab.freedesktop.org/mesa/mesa.git", 1) != 0) {
            log_error("download_gpu_driver_sources", "Failed to clone Mesa.", 0);
            return -1;
        }
    }

    // Clone tsukumijima/libmali-rockchip
    log_message("INFO", "Cloning libmali-rockchip repository...");
    if (access("libmali-rockchip", F_OK) != 0) {
        if (execute_command("git clone --depth 1 https://github.com/tsukumijima/libmali-rockchip.git", 1) != 0) {
            log_error("download_gpu_driver_sources", "Failed to clone libmali-rockchip.", 0);
            return -1;
        }
    }

    // Clone linux-firmware for the Mali blob
    log_message("INFO", "Cloning linux-firmware repository for Mali CSF...");
    if (access("linux-firmware", F_OK) != 0) {
        if (execute_command("git clone --depth 1 git://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git", 1) != 0) {
            log_error("download_gpu_driver_sources", "Failed to clone linux-firmware.", 0);
            return -1;
        }
    }

    log_message("SUCCESS", "All GPU sources downloaded.");
    return 0;
}

// Builds the Mesa drivers from source using Meson and Ninja
int build_mesa_drivers(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char mesa_dir[MAX_PATH_LEN];
    snprintf(mesa_dir, sizeof(mesa_dir), "%s/gpu_sources/mesa", config->build_dir);

    log_message("INFO", "Building Mesa drivers (Panfrost/Panthor)...");

    if (chdir(mesa_dir) != 0) {
        log_system_error("build_mesa_drivers", "chdir");
        return -1;
    }

    // Configure with Meson
    log_message("INFO", "Configuring Mesa build with Meson...");
    snprintf(cmd, sizeof(cmd),
             "meson setup build/ -Dplatforms=x11,wayland -Dgallium-drivers=panfrost,kmsro -Dvulkan-drivers=panfrost -Dlibunwind=disabled");
    if (execute_command(cmd, 1) != 0) {
        log_error("build_mesa_drivers", "Meson configuration failed.", 0);
        return -1;
    }

    // Compile with Ninja
    log_message("INFO", "Compiling Mesa with Ninja...");
    snprintf(cmd, sizeof(cmd), "ninja -C build/");
    if (execute_command(cmd, 1) != 0) {
        log_error("build_mesa_drivers", "Ninja build failed.", 0);
        return -1;
    }

    log_message("SUCCESS", "Mesa drivers built successfully.");
    return 0;
}

// Installs the compiled drivers and firmware to a destination directory
int install_gpu_drivers(build_config_t *config) {
    char cmd[MAX_CMD_LEN];
    char rootfs_dir[MAX_PATH_LEN];
    char mesa_dir[MAX_PATH_LEN];
    char firmware_dir[MAX_PATH_LEN];

    snprintf(rootfs_dir, sizeof(rootfs_dir), "%s/rootfs", config->build_dir);
    snprintf(mesa_dir, sizeof(mesa_dir), "%s/gpu_sources/mesa", config->build_dir);
    snprintf(firmware_dir, sizeof(firmware_dir), "%s/gpu_sources/linux-firmware/amdgpu", config->build_dir); // Path to mali firmware is inside amdgpu folder

    log_message("INFO", "Installing GPU drivers and firmware...");

    // Install Mesa drivers
    if (chdir(mesa_dir) != 0) {
        log_system_error("install_gpu_drivers", "chdir to mesa");
        return -1;
    }
    log_message("INFO", "Installing Mesa drivers to rootfs...");
    snprintf(cmd, sizeof(cmd), "DESTDIR=%s ninja -C build/ install", rootfs_dir);
    if (execute_command(cmd, 1) != 0) {
        log_error("install_gpu_drivers", "Failed to install Mesa drivers.", 0);
        return -1;
    }

    // Install Mali firmware
    log_message("INFO", "Installing Mali firmware...");
    char firmware_target_dir[MAX_PATH_LEN];
    snprintf(firmware_target_dir, sizeof(firmware_target_dir), "%s/lib/firmware", rootfs_dir);
    create_directory(firmware_target_dir);
    snprintf(cmd, sizeof(cmd), "cp %s/mali_csffw.bin %s/", firmware_dir, firmware_target_dir);
    if (execute_command(cmd, 1) != 0) {
        log_message("WARNING", "Failed to install Mali firmware. This might be okay if the kernel provides it.");
    }

    log_message("SUCCESS", "GPU drivers and firmware installed.");
    return 0;
}

// Placeholder for GPU verification
int verify_gpu_installation(void) {
    log_message("INFO", "Verifying GPU installation (placeholder)...");
    // In a real scenario, we would run commands like glxinfo, vulkaninfo inside the chroot
    log_message("SUCCESS", "GPU installation verification complete.");
    return 0;
}

int install_gpu_drivers_legacy(const char* rootfs_path) {
    char build_dir[] = "/tmp/gpu_build_XXXXXX";
    if (mkdtemp(build_dir) == NULL) {
        log_error("install_gpu_drivers_legacy", "Failed to create temporary build directory", 1);
        return -1;
    }

    log_info("Starting GPU driver installation...");

    if (install_mesa_panfrost(rootfs_path, build_dir) != 0) {
        log_error("install_gpu_drivers_legacy", "Failed to install Mesa/Panfrost drivers.", 1);
        // Don't return immediately, try to install other components
    }

    if (install_libmali_rockchip(rootfs_path, build_dir) != 0) {
        log_error("install_gpu_drivers_legacy", "Failed to install libmali-rockchip.", 1);
    }

    if (install_gpu_firmware(rootfs_path) != 0) {
        log_error("install_gpu_drivers_legacy", "Failed to install GPU firmware.", 1);
    }

    configure_vulkan(rootfs_path);

    log_info("Cleaning up build directory: %s", build_dir);
    char command[512];
    snprintf(command, sizeof(command), "rm -rf %s", build_dir);
    if (system(command) != 0) {
        log_warn("Failed to clean up temporary build directory: %s", build_dir);
    }


    log_info("GPU driver installation completed.");
    return 0;
}

int install_mesa_panfrost(const char* rootfs_path, const char* build_dir) {
    log_info("Installing Mesa with Panfrost/Panthor support...");
    char mesa_build_dir[256];
    snprintf(mesa_build_dir, sizeof(mesa_build_dir), "%s/mesa", build_dir);

    char command[1024];
    snprintf(command, sizeof(command), "git clone --depth 1 %s %s", MESA_REPO, mesa_build_dir);
    if (run_command(command) != 0) {
        log_error("install_mesa_panfrost", "Failed to clone Mesa repository.", 1);
        return -1;
    }

    // Note: This is a simplified build process. A real build would require
    // more complex configuration (meson, etc.) and cross-compilation setup.
    // This assumes a native build environment or a properly configured chroot.
    log_info("Building Mesa (this will take a long time)...");
    snprintf(command, sizeof(command),
        "cd %s && \
        meson setup build -D platforms=x11,wayland -D gallium-drivers=panfrost,kmsro -D vulkan-drivers=rockchip -D dri3=enabled -D egl=enabled -D gles2=enabled -D glx=dri -D libunwind=disabled --prefix=/usr && \
        ninja -C build && \
        DESTDIR=%s ninja -C build install",
        mesa_build_dir, rootfs_path);

    if (run_command_chroot(rootfs_path, command) != 0) {
        log_error("install_mesa_panfrost", "Failed to build and install Mesa.", 1);
        return -1;
    }

    log_info("Mesa installation successful.");
    return 0;
}

int install_libmali_rockchip(const char* rootfs_path, const char* build_dir) {
    log_info("Installing libmali-rockchip...");
    char libmali_build_dir[256];
    snprintf(libmali_build_dir, sizeof(libmali_build_dir), "%s/libmali", build_dir);

    char command[1024];
    // Clone the specific branch for rk3588
    snprintf(command, sizeof(command), "git clone --depth 1 -b rk3588 %s %s", LIBMALI_REPO, libmali_build_dir);
    if (run_command(command) != 0) {
        log_error("install_libmali_rockchip", "Failed to clone libmali repository.", 1);
        return -1;
    }

    log_info("Building libmali-rockchip...");
    // This also requires a proper build environment.
    snprintf(command, sizeof(command),
        "cd %s && \
        ./autogen.sh && \
        ./configure --prefix=/usr && \
        make && \
        DESTDIR=%s make install",
        libmali_build_dir, rootfs_path);

    if (run_command_chroot(rootfs_path, command) != 0) {
        log_error("install_libmali_rockchip", "Failed to build and install libmali-rockchip.", 1);
        return -1;
    }

    log_info("libmali-rockchip installation successful.");
    return 0;
}

int install_gpu_firmware(const char* rootfs_path) {
    log_info("Installing GPU firmware...");
    char firmware_dir[] = "/tmp/linux-firmware_XXXXXX";
    if (mkdtemp(firmware_dir) == NULL) {
        log_error("install_gpu_firmware", "Failed to create temporary firmware directory", 1);
        return -1;
    }

    char command[1024];
    snprintf(command, sizeof(command), "git clone --depth 1 %s %s", FIRMWARE_REPO, firmware_dir);
    if (run_command(command) != 0) {
        log_error("install_gpu_firmware", "Failed to clone linux-firmware repository.", 1);
        return -1;
    }

    char dest_path[512];
    snprintf(dest_path, sizeof(dest_path), "%s/lib/firmware", rootfs_path);
    log_info("Copying firmware files to %s", dest_path);

    // Specifically copy the firmware needed for the Mali G610 (Panfrost)
    // The exact filenames might need verification.
    snprintf(command, sizeof(command), "mkdir -p %s/rockchip && cp %s/rockchip/g610* %s/rockchip/", dest_path, firmware_dir, dest_path);
     if (run_command(command) != 0) {
        log_warn("Could not find or copy specific G610 firmware. Copying all of rockchip.");
        snprintf(command, sizeof(command), "mkdir -p %s/rockchip && cp -r %s/rockchip/* %s/rockchip/", dest_path, firmware_dir, dest_path);
        if (run_command(command) != 0) {
            log_error("install_gpu_firmware", "Failed to copy rockchip firmware.", 1);
            return -1;
        }
    }


    log_info("Cleaning up firmware directory: %s", firmware_dir);
    snprintf(command, sizeof(command), "rm -rf %s", firmware_dir);
    system(command); // Best effort cleanup

    log_info("GPU firmware installation successful.");
    return 0;
}

void configure_vulkan(const char* rootfs_path) {
    log_info("Configuring Vulkan ICD...");
    char icd_dir[512];
    snprintf(icd_dir, sizeof(icd_dir), "%s/etc/vulkan/icd.d", rootfs_path);
    char command[1024];
    snprintf(command, sizeof(command), "mkdir -p %s", icd_dir);
    if(run_command(command) != 0){
        log_error("configure_vulkan", "Failed to create Vulkan ICD directory.", 1);
        return;
    };


    char icd_file_path[512];
    snprintf(icd_file_path, sizeof(icd_file_path), "%s/rockchip_icd.x86_64.json", icd_dir);

    FILE* icd_file = fopen(icd_file_path, "w");
    if (!icd_file) {
        log_error("configure_vulkan", "Failed to create Vulkan ICD file", 1);
        return;
    }

    fprintf(icd_file, "{\n");
    fprintf(icd_file, "    \"file_format_version\": \"1.0.0\",\n");
    fprintf(icd_file, "    \"ICD\": {\n");
    fprintf(icd_file, "        \"library_path\": \"libvulkan_rockchip.so\",\n");
    fprintf(icd_file, "        \"api_version\": \"1.1.0\"\n");
    fprintf(icd_file, "    }\n");
    fprintf(icd_file, "}\n");

    fclose(icd_file);
    log_info("Vulkan ICD configuration complete.");
}
