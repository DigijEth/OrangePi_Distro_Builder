#!/bin/bash

# This script installs all the necessary dependencies for building the
# Orange Pi 5 Plus custom Ubuntu image using the builder application.
# Run this script with sudo privileges: sudo ./install_dependencies.sh

# Exit immediately if a command exits with a non-zero status.
set -e

# Color codes for output
COLOR_GREEN="\033[32m"
COLOR_YELLOW="\033[33m"
COLOR_RESET="\033[0m"

echo -e "${COLOR_YELLOW}Starting dependency installation for the Orange Pi 5 Plus Builder...${COLOR_RESET}"

# Update package lists
echo "Updating package lists..."
apt-get update

# Array of packages to install
PACKAGES=(
    # Basic build tools
    build-essential
    gcc-aarch64-linux-gnu
    g++-aarch64-linux-gnu
    libncurses-dev
    gawk
    flex
    bison
    openssl
    libssl-dev
    dkms
    libelf-dev
    libudev-dev
    libpci-dev
    libiberty-dev
    autoconf
    llvm
    # Additional tools
    git
    wget
    curl
    bc
    rsync
    kmod
    cpio
    python3
    python3-pip
    device-tree-compiler
    # Ubuntu kernel build dependencies
    fakeroot
    kernel-package
    # Mali GPU and OpenCL/Vulkan support
    mesa-opencl-icd
    vulkan-tools
    vulkan-utils
    vulkan-validationlayers
    libvulkan-dev
    ocl-icd-opencl-dev
    opencl-headers
    clinfo
    # Media and hardware acceleration
    va-driver-all
    vdpau-driver-all
    mesa-va-drivers
    mesa-vdpau-drivers
    # Development libraries
    libegl1-mesa-dev
    libgles2-mesa-dev
    libgl1-mesa-dev
    libdrm-dev
    libgbm-dev
    libwayland-dev
    libx11-dev
    meson
    ninja-build
    # For rootfs creation
    debootstrap
    qemu-user-static
    binfmt-support
    parted
)

# Install all packages at once
echo "Installing required packages..."
DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends "${PACKAGES[@]}"

# Install additional Ubuntu kernel build dependencies
echo "Installing kernel-specific build dependencies..."
apt-get build-dep -y linux linux-image-unsigned-$(uname -r)

echo -e "${COLOR_GREEN}All dependencies have been successfully installed.${COLOR_RESET}"
echo "You are now ready to run the builder application."

