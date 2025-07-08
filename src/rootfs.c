#include "rootfs.h"
#include "logging.h"
#include "system_utils.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Build Ubuntu rootfs for Orange Pi 5 Plus
int build_rootfs(const char* rootfs_path) {
    log_info("Building Ubuntu %s rootfs for Orange Pi 5 Plus...", UBUNTU_VERSION);
    
    const char* target_path = rootfs_path ? rootfs_path : ROOTFS_PATH;
    char command[2048];
    
    // Create rootfs directory
    snprintf(command, sizeof(command), "mkdir -p %s", target_path);
    execute_command(command, 1);
    
    // Clean existing rootfs if present
    snprintf(command, sizeof(command), "rm -rf %s/*", target_path);
    execute_command(command, 1);
    
    // Install debootstrap if not available
    log_info("Ensuring debootstrap is available...");
    execute_command("apt update && apt install -y debootstrap ubuntu-keyring arch-test", 1);
    
    // Create base Ubuntu rootfs with Orange Pi optimizations
    log_info("Creating Ubuntu %s base system (this will take several minutes)...", UBUNTU_CODENAME);
    
    const char *ubuntu_components = "main,universe,restricted,multiverse";
    const char *essential_packages = "systemd,udev,kmod,initramfs-tools,openssh-server,sudo,nano,wget,curl,git,build-essential,software-properties-common,apt-transport-https,ca-certificates,gnupg,lsb-release,linux-firmware,network-manager,wpasupplicant";
    
    snprintf(command, sizeof(command),
        "debootstrap --arch=%s --components=%s --include=%s %s %s %s",
        UBUNTU_ARCH, ubuntu_components, essential_packages, UBUNTU_CODENAME, target_path, UBUNTU_MIRROR);
    
    if (execute_command(command, 1) != 0) {
        log_error("build_rootfs", "Failed to create base Ubuntu system", 1);
        return -1;
    }
    
    // Configure the rootfs for Orange Pi 5 Plus
    configure_orangepi_rootfs(target_path);
    
    // Install Orange Pi specific packages
    install_orangepi_packages(target_path);
    
    // Configure GPU drivers
    configure_gpu_drivers(target_path);
    
    log_info("Ubuntu rootfs build completed successfully");
    return 0;
}

// Configure rootfs specifically for Orange Pi 5 Plus
int configure_orangepi_rootfs(const char* rootfs_path) {
    log_info("Configuring rootfs for Orange Pi 5 Plus...");
    
    char command[2048];
    
    // Configure sources.list for all Ubuntu components
    snprintf(command, sizeof(command),
        "cat > %s/etc/apt/sources.list << 'EOF'\n"
        "deb %s %s main restricted universe multiverse\n"
        "deb %s %s-updates main restricted universe multiverse\n"
        "deb %s %s-backports main restricted universe multiverse\n"
        "deb %s %s-security main restricted universe multiverse\n"
        "EOF", 
        rootfs_path, UBUNTU_MIRROR, UBUNTU_CODENAME, UBUNTU_MIRROR, UBUNTU_CODENAME,
        UBUNTU_MIRROR, UBUNTU_CODENAME, UBUNTU_MIRROR, UBUNTU_CODENAME);
    execute_command(command, 1);
    
    // Configure hostname
    snprintf(command, sizeof(command),
        "echo 'orangepi5plus' > %s/etc/hostname", rootfs_path);
    execute_command(command, 1);
    
    // Configure hosts file
    snprintf(command, sizeof(command),
        "cat > %s/etc/hosts << 'EOF'\n"
        "127.0.0.1   localhost\n"
        "127.0.1.1   orangepi5plus\n"
        "::1         localhost ip6-localhost ip6-loopback\n"
        "ff02::1     ip6-allnodes\n"
        "ff02::2     ip6-allrouters\n"
        "EOF", rootfs_path);
    execute_command(command, 1);
    
    // Configure default user
    snprintf(command, sizeof(command),
        "chroot %s /bin/bash -c '"
        "useradd -m -s /bin/bash -G sudo,audio,video,plugdev,netdev,bluetooth orangepi && "
        "echo \"orangepi:orangepi\" | chpasswd && "
        "echo \"root:orangepi\" | chpasswd'", rootfs_path);
    execute_command(command, 1);
    
    // Configure network interfaces
    snprintf(command, sizeof(command),
        "cat > %s/etc/systemd/network/eth0.network << 'EOF'\n"
        "[Match]\n"
        "Name=eth0\n"
        "[Network]\n"
        "DHCP=yes\n"
        "EOF", rootfs_path);
    execute_command(command, 1);
    
    // Enable essential services
    snprintf(command, sizeof(command),
        "chroot %s /bin/bash -c '"
        "systemctl enable systemd-networkd && "
        "systemctl enable systemd-resolved && "
        "systemctl enable ssh && "
        "systemctl enable NetworkManager'", rootfs_path);
    execute_command(command, 1);
    
    log_info("Orange Pi rootfs configuration completed");
    return 0;
}

// Install Orange Pi 5 Plus specific packages
int install_orangepi_packages(const char* rootfs_path) {
    log_info("Installing Orange Pi 5 Plus specific packages...");
    
    char command[2048];
    
    // Update package lists
    snprintf(command, sizeof(command),
        "chroot %s /bin/bash -c 'apt update'", rootfs_path);
    execute_command(command, 1);
    
    // Install hardware support packages
    snprintf(command, sizeof(command),
        "chroot %s /bin/bash -c '"
        "apt install -y "
        "linux-firmware-raspi2 "
        "wireless-regdb "
        "wpasupplicant "
        "bluetooth "
        "bluez "
        "bluez-tools "
        "network-manager "
        "avahi-daemon "
        "i2c-tools "
        "spi-tools "
        "gpio-utils "
        "python3-rpi.gpio "
        "device-tree-compiler'", rootfs_path);
    execute_command(command, 1);
    
    // Install multimedia packages
    snprintf(command, sizeof(command),
        "chroot %s /bin/bash -c '"
        "apt install -y "
        "alsa-utils "
        "pulseaudio "
        "pulseaudio-utils "
        "pavucontrol "
        "gstreamer1.0-tools "
        "gstreamer1.0-plugins-base "
        "gstreamer1.0-plugins-good "
        "gstreamer1.0-plugins-bad "
        "gstreamer1.0-plugins-ugly "
        "gstreamer1.0-vaapi "
        "ffmpeg "
        "v4l-utils'", rootfs_path);
    execute_command(command, 1);
    
    // Install development tools
    snprintf(command, sizeof(command),
        "chroot %s /bin/bash -c '"
        "apt install -y "
        "gcc-aarch64-linux-gnu "
        "g++-aarch64-linux-gnu "
        "cmake "
        "ninja-build "
        "pkg-config "
        "autotools-dev "
        "autoconf "
        "automake "
        "libtool "
        "python3-dev "
        "python3-pip "
        "nodejs "
        "npm'", rootfs_path);
    execute_command(command, 1);
    
    log_info("Orange Pi packages installed successfully");
    return 0;
}

// Configure GPU drivers for Mali G610
int configure_gpu_drivers(const char* rootfs_path) {
    log_info("Configuring Mali G610 GPU drivers...");
    
    char command[2048];
    
    // Install Mesa with Panfrost drivers
    snprintf(command, sizeof(command),
        "chroot %s /bin/bash -c '"
        "apt install -y "
        "mesa-utils "
        "mesa-vulkan-drivers "
        "mesa-va-drivers "
        "mesa-vdpau-drivers "
        "libgl1-mesa-dri "
        "libglx-mesa0 "
        "libgles2-mesa "
        "libegl1-mesa "
        "libvulkan1 "
        "vulkan-tools "
        "vulkan-utils "
        "clinfo "
        "opencl-headers "
        "libdrm2 "
        "libgbm1 "
        "vainfo "
        "vdpauinfo'", rootfs_path);
    execute_command(command, 1);
    
    // Configure GPU performance governor
    snprintf(command, sizeof(command),
        "cat > %s/etc/systemd/system/gpu-performance.service << 'EOF'\n"
        "[Unit]\n"
        "Description=Set GPU Performance Governor\n"
        "After=multi-user.target\n"
        "[Service]\n"
        "Type=oneshot\n"
        "ExecStart=/bin/bash -c 'echo performance > /sys/class/devfreq/fb000000.gpu/governor || true'\n"
        "RemainAfterExit=yes\n"
        "[Install]\n"
        "WantedBy=multi-user.target\n"
        "EOF", rootfs_path);
    execute_command(command, 1);
    
    // Enable GPU performance service
    snprintf(command, sizeof(command),
        "chroot %s /bin/bash -c 'systemctl enable gpu-performance'", rootfs_path);
    execute_command(command, 1);
    
    // Configure Xorg for Mali GPU
    snprintf(command, sizeof(command),
        "mkdir -p %s/etc/X11/xorg.conf.d && "
        "cat > %s/etc/X11/xorg.conf.d/20-mali.conf << 'EOF'\n"
        "Section \"Device\"\n"
        "    Identifier \"Mali GPU\"\n"
        "    Driver \"modesetting\"\n"
        "    Option \"AccelMethod\" \"glamor\"\n"
        "    Option \"DRI\" \"3\"\n"
        "EndSection\n"
        "EOF", rootfs_path, rootfs_path);
    execute_command(command, 1);
    
    log_info("GPU drivers configured successfully");
    return 0;
}

// Legacy wrapper for compatibility
int build_ubuntu_rootfs(build_config_t *config) {
    char rootfs_dir[MAX_PATH_LEN];
    char cmd[MAX_CMD_LEN];
    
    if (!config) {
        log_error("build_ubuntu_rootfs", "Configuration is NULL", 0);
        return -1;
    }
    
    snprintf(rootfs_dir, sizeof(rootfs_dir), "%s/rootfs", config->build_dir);
    
    // Create base Ubuntu rootfs using debootstrap
    log_info("Creating Ubuntu 25.04 base rootfs...");
    snprintf(cmd, sizeof(cmd), 
        "debootstrap --arch=arm64 --variant=minbase noble %s http://ports.ubuntu.com/ubuntu-ports/", 
        rootfs_dir);

    if (execute_command(cmd, 1) != 0) {
        log_error("build_ubuntu_rootfs", "Failed to create base rootfs with debootstrap", 0);
        return -1;
    }

    // Chroot into the new filesystem to configure it
    char sources_list_path[MAX_PATH_LEN];
    snprintf(sources_list_path, sizeof(sources_list_path), "%s/etc/apt/sources.list", rootfs_dir);

    FILE *sources_file = fopen(sources_list_path, "w");
    if (!sources_file) {
        log_system_error("build_ubuntu_rootfs", "fopen sources.list");
        return -1;
    }

    // Write comprehensive Ubuntu sources.list with all release channels
    fprintf(sources_file,
            "deb http://ports.ubuntu.com/ubuntu-ports noble main restricted universe multiverse\n"
            "deb-src http://ports.ubuntu.com/ubuntu-ports noble main restricted universe multiverse\n"
            "deb http://ports.ubuntu.com/ubuntu-ports noble-updates main restricted universe multiverse\n"
            "deb-src http://ports.ubuntu.com/ubuntu-ports noble-updates main restricted universe multiverse\n"
            "deb http://ports.ubuntu.com/ubuntu-ports noble-backports main restricted universe multiverse\n"
            "deb-src http://ports.ubuntu.com/ubuntu-ports noble-backports main restricted universe multiverse\n"
            "deb http://ports.ubuntu.com/ubuntu-ports noble-security main restricted universe multiverse\n"
            "deb-src http://ports.ubuntu.com/ubuntu-ports noble-security main restricted universe multiverse\n");

    fclose(sources_file);

    // Set up basic system configuration
    log_info("Configuring basic system settings...");

    // Create hostname
    char hostname_path[MAX_PATH_LEN];
    snprintf(hostname_path, sizeof(hostname_path), "%s/etc/hostname", rootfs_dir);
    FILE *hostname_file = fopen(hostname_path, "w");
    if (hostname_file) {
        fprintf(hostname_file, "orangepi5plus");
        fclose(hostname_file);
    }

    // Create hosts file
    char hosts_path[MAX_PATH_LEN];
    snprintf(hosts_path, sizeof(hosts_path), "%s/etc/hosts", rootfs_dir);
    FILE *hosts_file = fopen(hosts_path, "w");
    if (hosts_file) {
        fprintf(hosts_file, "127.0.0.1 localhost\n127.0.1.1 orangepi5plus\n");
        fclose(hosts_file);
    }

    log_info("Ubuntu 25.04 root filesystem created successfully");
    return 0;
}

// Install system packages for Orange Pi 5 Plus
int install_system_packages(build_config_t *config) {
    if (!config) {
        log_error("install_system_packages", "Configuration is NULL", 0);
        return -1;
    }

    log_info("Installing system packages for Orange Pi 5 Plus...");

    // Update package lists
    if (execute_command("apt update", 1) != 0) {
        log_error("install_system_packages", "Failed to update package lists", 0);
        return -1;
    }

    // Install essential build tools
    const char *build_packages = "build-essential git cmake ninja-build pkg-config "
                                "gcc-aarch64-linux-gnu g++-aarch64-linux-gnu "
                                "device-tree-compiler u-boot-tools "
                                "bc bison flex libssl-dev libncurses5-dev "
                                "parted kpartx dosfstools debootstrap qemu-user-static";

    char cmd[MAX_CMD_LEN];
    snprintf(cmd, sizeof(cmd), "apt install -y %s", build_packages);

    if (execute_command(cmd, 1) != 0) {
        log_error("install_system_packages", "Failed to install build packages", 0);
        return -1;
    }

    log_info("System packages installed successfully");
    return 0;
}

// Configure system services for Orange Pi 5 Plus
int configure_system_services(build_config_t *config) {
    if (!config) {
        log_error("configure_system_services", "Configuration is NULL", 0);
        return -1;
    }

    log_info("Configuring system services for Orange Pi 5 Plus...");

    // Enable required services
    const char *services[] = {
        "ssh",
        "systemd-networkd",
        "systemd-resolved",
        "NetworkManager",
        NULL
    };

    char cmd[MAX_CMD_LEN];
    for (int i = 0; services[i] != NULL; i++) {
        snprintf(cmd, sizeof(cmd), "systemctl enable %s", services[i]);
        if (execute_command(cmd, 1) != 0) {
            log_warn("Failed to enable service");
        }
    }

    log_info("System services configured successfully");
    return 0;
}

void rootfs_menu(void) {
    char choice[10];
    int choice_int;

    while (1) {
        printf("\n%s%s--- Root Filesystem Menu ---%s\n", COLOR_BOLD, COLOR_YELLOW, COLOR_RESET);
        printf("1. Build Ubuntu RootFS\n");
        printf("2. Configure System Services\n");
        printf("3. Install System Packages\n");
        printf("4. Return to Main Menu\n");
        printf("Enter your choice: ");

        if (fgets(choice, sizeof(choice), stdin) == NULL) continue;
        choice_int = atoi(choice);

        switch (choice_int) {
            case 1:
                build_ubuntu_rootfs(&g_build_config);
                break;
            case 2:
                configure_system_services(&g_build_config);
                break;
            case 3:
                install_system_packages(&g_build_config);
                break;
            case 4:
                return;
            default:
                log_warn("Invalid choice. Please try again.");
        }
    }
}
