# Orange Pi 5 Plus Ubuntu Builder

A comprehensive tool for building custom Ubuntu images for Orange Pi 5 Plus single-board computers.

## Features

- **Custom Kernel Building**: Compile Linux kernels optimized for Orange Pi 5 Plus with RK3588 SoC
- **U-Boot Support**: Build and install U-Boot bootloader with Orange Pi specific configurations
- **Ubuntu Root Filesystem**: Create complete Ubuntu systems using debootstrap
- **GPU Driver Support**: Install Mali G610 GPU drivers with Panfrost/Mesa support
- **Hardware Acceleration**: Configure Vulkan and OpenCL support for GPU acceleration
- **Bootable Images**: Generate ready-to-flash SD card images
- **Interactive Interface**: User-friendly menu-driven interface
- **Dependency Management**: Automatic installation and verification of build dependencies

## Requirements

- Ubuntu/Debian host system
- Root privileges (sudo access)
- At least 20GB free disk space
- Internet connection for downloading sources
- 4GB+ RAM recommended for kernel compilation

## Usage

Run the tool with root privileges:

```bash
sudo orangepi-ubuntu-builder
```

The interactive menu will guide you through:

1. **Full Build**: Complete automated build process
2. **Build Kernel**: Compile custom kernel only
3. **Build U-Boot**: Compile bootloader only
4. **Build RootFS**: Create Ubuntu filesystem only
5. **Install GPU Drivers**: Set up Mali GPU acceleration
6. **Create Bootable Image**: Generate SD card image
7. **Manage Dependencies**: Install/check build requirements

## Configuration

Edit `/etc/orangepi-ubuntu-builder/builder.conf` to customize build parameters:

```bash
KERNEL_VERSION=6.1
BUILD_JOBS=4
ENABLE_GPU=1
```

## Output

- Built kernels: `/tmp/kernel_build/`
- Root filesystem: `/tmp/rootfs_build/`
- Bootable images: `/tmp/orangepi_ubuntu_*.img`
- Build logs: `/tmp/kernel_build.log`

## Hardware Support

- **SoC**: Rockchip RK3588
- **GPU**: Mali G610 MP4 with Panfrost driver
- **Architecture**: ARM64 (AArch64)
- **Board**: Orange Pi 5 Plus

## Troubleshooting

Check the build log for detailed error information:
```bash
tail -f /tmp/kernel_build.log
```

Common issues:
- Insufficient disk space
- Missing build dependencies
- Network connectivity problems
- Permission issues (ensure running as root)

## License

GPL-3.0 License. See `/usr/share/doc/orangepi-ubuntu-builder/copyright` for details.

## Support

- Documentation: `/usr/share/doc/orangepi-ubuntu-builder/`
- Man page: `man orangepi-ubuntu-builder`
- Issues: Report bugs to the project repository
