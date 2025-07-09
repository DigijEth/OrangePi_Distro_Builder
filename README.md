# Orange Pi 5 Plus Ubuntu Builder

A comprehensive C application for building custom Ubuntu images and firmware for Orange Pi 5 Plus single-board computers with RK3588 SoC.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Build Status](https://img.shields.io/badge/Build-Passing-green.svg)]()
[![Version](https://img.shields.io/badge/Version-2.0.0-orange.svg)]()

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Detailed Usage](#detailed-usage)
- [Configuration](#configuration)
- [Build Process](#build-process)
- [Output Files](#output-files)
- [Hardware Support](#hardware-support)
- [Troubleshooting](#troubleshooting)
- [Development](#development)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)

## Overview

The Orange Pi 5 Plus Ubuntu Builder is a native C application designed to create complete, custom Ubuntu systems for Orange Pi 5 Plus hardware. Unlike simple image flashers, this tool builds everything from source - kernels, bootloaders, root filesystems, and GPU drivers - providing maximum customization and optimization for the RK3588 SoC.

### What Makes This Different
- **Source-based builds**: Compiles everything from source code for optimal performance
- **Complete firmware generation**: Creates both .bin firmware files and .img bootable images
- **GPU optimization**: Multiple Mali G610 driver options including custom patches
- **Frontend integration**: Built-in support for gaming and media center environments
- **Professional tooling**: Enterprise-grade build system with comprehensive logging

## Features

### 🔧 **Core Build System**
- **Custom Kernel Building**: Linux kernels optimized for Orange Pi 5 Plus with RK3588-specific patches
- **U-Boot Compilation**: Bootloader building with Orange Pi configurations and firmware generation
- **Ubuntu Root Filesystem**: Complete Ubuntu 22.04 systems created with debootstrap
- **Cross-compilation Support**: Full ARM64 toolchain integration for x86_64 host systems

### 🎮 **Gaming & Media**
- **RetroArch Integration**: Pre-configured retro gaming frontend with optimized settings
- **EmulationStation Support**: Popular retro gaming interface with theme support
- **Lakka Integration**: Lightweight gaming-focused Linux distribution option
- **Kodi Media Center**: Full media center installation with hardware acceleration

### 🖥️ **GPU & Hardware Acceleration**
- **Mali G610 MP4 Support**: Native driver support for RK3588 GPU
- **Multiple Driver Options**:
  - Panfrost (default open-source driver)
  - Armbian GPU patches for enhanced compatibility
  - Framework for custom driver development
- **Vulkan API Support**: Hardware-accelerated graphics for modern applications
- **OpenCL Support**: GPU compute acceleration for compatible software

### 🛠️ **Developer Tools**
- **Interactive Menu System**: User-friendly interface for all operations
- **Automated Dependency Management**: Handles complex build requirements automatically
- **Comprehensive Logging**: Detailed build process tracking with error reporting
- **Modular Architecture**: Extensible C codebase for additional features
- **Configuration Management**: File-based settings for build customization

## System Requirements

### Host System
- **Operating System**: Ubuntu 20.04+ or Debian 11+ (x86_64)
- **Privileges**: Root access (sudo) required for system operations
- **Disk Space**: Minimum 20GB free space (40GB+ recommended)
- **Memory**: 4GB RAM minimum (8GB+ recommended for parallel builds)
- **Network**: Internet connection for downloading sources and packages

### Build Dependencies
The following packages are automatically managed by the installer:
```bash
build-essential gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
libncurses-dev gawk flex bison openssl libssl-dev git wget curl
bc rsync python3 debootstrap fakeroot device-tree-compiler
mesa-opencl-icd vulkan-tools libvulkan-dev opencl-headers
```

### Target Hardware
- **Board**: Orange Pi 5 Plus
- **SoC**: Rockchip RK3588 (ARM64)
- **GPU**: Mali G610 MP4
- **Storage**: SD card (16GB+) or eMMC module
- **RAM**: 4GB, 8GB, 16GB, or 32GB variants supported

## Installation

### Method 1: Debian Package (Recommended)
```bash
# Download the latest release
wget https://github.com/orangepi/orangepi-ubuntu-builder/releases/download/v2.0.0/orangepi-ubuntu-builder_2.0.0-1_amd64.deb

# Install the package
sudo dpkg -i orangepi-ubuntu-builder_2.0.0-1_amd64.deb

# Fix any dependency issues
sudo apt-get install -f
```

### Method 2: Build from Source
```bash
# Clone the repository
git clone https://github.com/orangepi/orangepi-ubuntu-builder.git
cd orangepi-ubuntu-builder

# Install build dependencies
sudo ./install_dependencies.sh

# Build the application
make clean && make

# Install system-wide (optional)
sudo make install
```
### Basic Usage
```bash
# Launch the builder with root privileges
sudo orangepi-ubuntu-builder
```

### Menu Navigation
1. **Full Build** - Complete automated build process
2. **Build Kernel** - Compile custom kernel only
3. **Build U-Boot** - Compile bootloader only
4. **Build RootFS** - Create Ubuntu filesystem only
5. **GPU/Gaming Setup** - Install GPU drivers and gaming frontends
6. **Create Images** - Generate bootable SD card images
7. **System Configuration** - Advanced build settings

### Quick Build Example
```bash
# Start the application
sudo orangepi-ubuntu-builder

# Select option 1 for "Full Build"
# Choose your preferred configuration:
# - Gaming optimized (includes RetroArch, EmulationStation)
# - Media center (includes Kodi)
# - Developer build (includes development tools)
# - Server optimized (minimal, headless)

# The build process will:
# 1. Check and install dependencies
# 2. Download kernel and U-Boot sources
# 3. Apply Orange Pi patches
# 4. Compile kernel and U-Boot
# 5. Create Ubuntu root filesystem
# 6. Install GPU drivers and selected frontends
# 7. Generate bootable image
```

## Configuration

### System Configuration
```bash
# Global configuration:
/etc/orangepi-ubuntu-builder/builder.conf

# User configuration (per-build):
~/.config/orangepi-ubuntu-builder/build.conf

# Patch directories:
/etc/orangepi-ubuntu-builder/patches/kernel/
/etc/orangepi-ubuntu-builder/patches/uboot/
```

### Build Configuration File
```bash
# Edit: /etc/orangepi-ubuntu-builder/builder.conf

# Kernel settings
KERNEL_VERSION=6.1
KERNEL_CONFIG=rockchip_linux_defconfig
ENABLE_CUSTOM_PATCHES=1

# Build settings
BUILD_JOBS=4                    # Number of parallel compilation jobs
CROSS_COMPILE=aarch64-linux-gnu-
TARGET_ARCH=arm64

# GPU settings
ENABLE_GPU=1
INSTALL_MESA=1
INSTALL_VULKAN=1
INSTALL_OPENCL=1

# Frontend settings
INSTALL_RETROARCH=1
INSTALL_EMULATIONSTATION=1
INSTALL_KODI=1

# Image settings
IMAGE_SIZE=8GB                  # Final image size
COMPRESS_IMAGE=1               # Compress final image
```

## Output Files

### Build Artifacts Location
```
/tmp/orangepi_build/
├── kernel/                     # Kernel source and build artifacts
│   ├── linux/                 # Kernel source code
│   ├── Image                  # Compiled kernel image
│   ├── rk3588-orangepi-5-plus.dtb  # Device tree blob
│   └── modules/               # Kernel modules
├── uboot/                     # U-Boot source and binaries
│   ├── u-boot/               # U-Boot source code
│   ├── u-boot-rockchip.bin   # Main U-Boot binary
│   ├── idbloader.img          # Initial boot loader
│   └── u-boot.itb            # U-Boot FIT image
├── rootfs/                    # Ubuntu root filesystem
│   ├── bin/, usr/, etc/       # Standard Linux directories
│   └── [complete Ubuntu system]
├── gpu_sources/               # GPU driver sources
│   ├── mesa/                  # Mesa 3D graphics library
│   └── linux-firmware/       # GPU firmware files
└── output/                    # Final images and installation files
    ├── orangepi_ubuntu_*.img  # Bootable image file
    ├── flash-uboot.sh         # U-Boot installation script
    └── checksums.txt          # File verification checksums
```

### Installation Scripts
```bash
# Flash image to SD card:
sudo dd if=/tmp/orangepi_build/output/orangepi_ubuntu_*.img of=/dev/sdX bs=4M status=progress

# Install U-Boot only:
sudo /tmp/orangepi_build/output/flash-uboot.sh /dev/sdX

# Verify image integrity:
sha256sum -c /tmp/orangepi_build/output/checksums.txt
```

## Hardware Support

### Supported Orange Pi Models
- **Orange Pi 5 Plus** (primary target)
- Orange Pi 5 (experimental support)
- Future RK3588-based models (planned)

### RK3588 SoC Features
```
CPU: ARM Cortex-A76 (4x) + Cortex-A55 (4x)
GPU: Mali G610 MP4
NPU: 6 TOPS AI acceleration
Video: 8K decode, 4K encode
Memory: Up to 32GB LPDDR4/5
Storage: eMMC, SATA, NVMe, SD card
```

## Troubleshooting

### Common Issues

#### Build Failures
```bash
# Issue: "No space left on device"
# Solution: Ensure 20GB+ free space
df -h /tmp
# Clean previous builds:
sudo rm -rf /tmp/orangepi_build/*

# Issue: "Cross-compiler not found"
# Solution: Install cross-compilation tools
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

#### GPU Driver Issues
```bash
# Issue: "GPU not detected"
# Check: Verify driver installation
lsmod | grep panfrost
dmesg | grep mali

# Issue: "3D acceleration not working"
# Check: Test GPU functionality
glxinfo | grep renderer
vulkaninfo --summary
```

### Getting Help
1. **Check the logs**: Always review `/tmp/orangepi_build.log` first
2. **Verify hardware**: Ensure Orange Pi 5 Plus compatibility
3. **Test SD card**: Try a different, high-quality SD card
4. **Serial console**: Connect serial adapter for boot debugging
5. **Community support**: Join Orange Pi forums and communities

## Development

### Project Structure
```
orangepi-ubuntu-builder/
├── src/                       # Source code modules
│   ├── auth.c/h              # Authentication and API access
│   ├── dependencies.c/h      # Dependency management
│   ├── gaming.c/h            # Gaming frontend integration
│   ├── gpu.c/h               # GPU driver management
│   ├── image.c/h             # Image creation and partitioning
│   ├── kernel.c/h            # Kernel building and configuration
│   ├── logging.c/h           # Logging and error handling
│   ├── rootfs.c/h            # Root filesystem creation
│   ├── system_utils.c/h      # System utilities and commands
│   └── uboot.c/h             # U-Boot building and installation
├── debian/                   # Debian package configuration
├── config/                   # Configuration files and templates
├── builder.c                 # Main application entry point
├── Makefile                  # Build system configuration
├── CLAUDE.md                 # Development documentation
└── README.md                 # This file
```

### Building from Source
```bash
# Clone repository:
git clone https://github.com/orangepi/orangepi-ubuntu-builder.git
cd orangepi-ubuntu-builder

# Install development dependencies:
sudo apt install build-essential git

# Build application:
make clean && make

# Install for development:
sudo make install PREFIX=/usr/local
```

## Contributing

We welcome contributions to the Orange Pi 5 Plus Ubuntu Builder! Here's how you can help:

### Ways to Contribute
- **Bug Reports**: Report issues with detailed reproduction steps
- **Feature Requests**: Suggest new functionality or improvements
- **Code Contributions**: Submit patches and new features
- **Documentation**: Improve documentation and examples
- **Testing**: Test on different hardware configurations
- **Community Support**: Help other users in forums and issues

### Contribution Process
1. **Fork** the repository on GitHub
2. **Create** a feature branch: `git checkout -b feature/amazing-feature`
3. **Make** your changes with appropriate tests
4. **Commit** with descriptive messages: `git commit -m 'Add amazing feature'`
5. **Push** to your branch: `git push origin feature/amazing-feature`
6. **Open** a Pull Request with detailed description

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

### GPL-3.0 License Summary
- **Freedom to use**: Use the software for any purpose
- **Freedom to study**: Access and study the source code
- **Freedom to modify**: Make changes and improvements
- **Freedom to distribute**: Share the software and your modifications
- **Copyleft**: Derivative works must also be GPL-3.0 licensed

### Third-Party Components
- **Linux Kernel**: GPL-2.0 (with GPL-2.0+ compatibility)
- **U-Boot**: GPL-2.0+
- **Mesa**: MIT License
- **Ubuntu Packages**: Various licenses (mostly GPL, MIT, Apache)

## Acknowledgments

This project builds upon the excellent work of many open-source projects and communities. We are grateful to all contributors and maintainers of the following projects:

### Core Operating System & Kernel
- **[Linux Kernel](https://kernel.org/)** - The foundation of our custom builds
- **[Ubuntu](https://ubuntu.com/)** - Base operating system and package ecosystem
- **[Debian](https://www.debian.org/)** - Package management and build system inspiration
- **[Armbian](https://www.armbian.com/)** - ARM board support and GPU patches
- **[Orange Pi Linux](https://github.com/orangepi-xunlong/linux-orangepi)** - Hardware-specific kernel patches and drivers

### Bootloader & Low-Level System
- **[U-Boot](https://www.denx.de/wiki/U-Boot)** - Universal bootloader for embedded systems
- **[Orange Pi U-Boot](https://github.com/orangepi-xunlong/u-boot-orangepi)** - Orange Pi specific U-Boot configurations
- **[Rockchip U-Boot](https://github.com/rockchip-linux/u-boot)** - RK3588 SoC bootloader support
- **[Device Tree Compiler](https://git.kernel.org/pub/scm/utils/dtc/dtc.git)** - Hardware description compilation

### GPU Drivers & Graphics
- **[Mesa 3D](https://mesa3d.org/)** - Open-source graphics drivers including Panfrost
- **[Panfrost Driver](https://docs.mesa3d.org/drivers/panfrost.html)** - Open-source Mali GPU driver
- **[ARM Mali Driver](https://developer.arm.com/tools-and-software/graphics-and-gaming/mali-drivers)** - Proprietary Mali GPU drivers
- **[Linux Firmware](https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git)** - GPU firmware binaries
- **[libdrm](https://gitlab.freedesktop.org/mesa/drm)** - Direct Rendering Manager library

### Gaming & Media Frontends
- **[RetroArch](https://www.retroarch.com/)** - Cross-platform retro gaming frontend
  - GitHub: [libretro/RetroArch](https://github.com/libretro/RetroArch)
- **[EmulationStation](https://emulationstation.org/)** - Popular retro gaming interface
  - GitHub: [RetroPie/EmulationStation](https://github.com/RetroPie/EmulationStation)
- **[Lakka](https://www.lakka.tv/)** - Lightweight retro gaming Linux distribution
  - GitHub: [libretro/Lakka-LibreELEC](https://github.com/libretro/Lakka-LibreELEC)
- **[Kodi](https://kodi.tv/)** - Open-source media center software
  - GitHub: [xbmc/xbmc](https://github.com/xbmc/xbmc)
- **[Box86/Box64](https://box86.org/)** - x86/x64 emulation on ARM
  - GitHub: [ptitSeb/box86](https://github.com/ptitSeb/box86), [ptitSeb/box64](https://github.com/ptitSeb/box64)

### Build System & Tools
- **[GNU Make](https://www.gnu.org/software/make/)** - Build automation tool
- **[GCC](https://gcc.gnu.org/)** - GNU Compiler Collection for cross-compilation
- **[Debootstrap](https://wiki.debian.org/Debootstrap)** - Debian/Ubuntu system bootstrapping
- **[QEMU](https://www.qemu.org/)** - Emulation for cross-architecture builds
- **[Meson](https://mesonbuild.com/)** - Build system for Mesa and other components
- **[Ninja](https://ninja-build.org/)** - Fast build system backend

### Hardware Acceleration APIs
- **[Vulkan](https://www.vulkan.org/)** - Low-level graphics and compute API
- **[OpenCL](https://www.khronos.org/opencl/)** - Cross-platform parallel computing
- **[OpenGL ES](https://www.khronos.org/opengles/)** - Graphics API for embedded systems
- **[V4L2](https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/v4l2.html)** - Video4Linux2 API for media devices

### Development Tools & Libraries
- **[Git](https://git-scm.com/)** - Version control system
- **[pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)** - Library configuration tool
- **[ncurses](https://invisible-island.net/ncurses/)** - Terminal user interface library
- **[OpenSSL](https://www.openssl.org/)** - Cryptography and SSL/TLS toolkit

### Hardware Vendors & Documentation
- **[Orange Pi](http://www.orangepi.org/)** - Single-board computer hardware and community
- **[Rockchip](https://www.rock-chips.com/)** - RK3588 SoC design and documentation
- **[ARM](https://www.arm.com/)** - Mali GPU architecture and documentation
- **[Khronos Group](https://www.khronos.org/)** - Graphics and compute API specifications

### Community Projects & Distributions
- **[Manjaro ARM](https://manjaro.org/download/#ARM)** - ARM Linux distribution insights
- **[Arch Linux ARM](https://archlinuxarm.org/)** - ARM-specific Linux distribution
- **[Buildroot](https://buildroot.org/)** - Embedded Linux build system inspiration
- **[Yocto Project](https://www.yoctoproject.org/)** - Custom Linux distribution framework
- **[OpenWrt](https://openwrt.org/)** - Embedded Linux router firmware project

### Special Recognition
- **[Orange Pi Community Forums](https://forums.orangepi.org/)** - Hardware support, testing, and user feedback
- **[Armbian Community](https://forum.armbian.com/)** - ARM board expertise and patch contributions
- **[LibreELEC](https://libreelec.tv/)** - Media center distribution architecture insights
- **[RetroPie](https://retropie.org.uk/)** - Retro gaming setup and configuration knowledge
- **[Pine64 Community](https://forum.pine64.org/)** - ARM development best practices

### Code Attribution
This project incorporates code, configurations, and techniques from:
- **Armbian build scripts** - GPU driver installation and hardware optimization
- **Orange Pi official images** - Hardware-specific configurations and patches
- **Mesa build configurations** - GPU driver compilation for ARM64
- **U-Boot Orange Pi configs** - Bootloader configurations and device trees
- **Ubuntu ARM builds** - Package selection and system configuration
- **RetroArch ARM configs** - Gaming frontend optimization for ARM hardware

We extend our heartfelt gratitude to all developers, maintainers, and communities who have made this project possible. The open-source ecosystem's collaborative spirit enables projects like this to exist and thrive.

### Contributing Back
We are committed to contributing improvements back to the upstream projects where possible. Our contributions include:
- Bug reports and testing feedback
- Hardware-specific patches and configurations  
- Documentation improvements
- Community support and knowledge sharing

If you are a maintainer of any of the above projects and have suggestions or concerns about our usage, please reach out through our GitHub issues.
