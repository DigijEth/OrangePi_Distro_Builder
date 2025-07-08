#!/bin/bash

# Simple Debian package builder for Orange Pi Ubuntu Builder
# This script creates a .deb package without requiring debhelper or checkinstall

set -e

echo "Orange Pi Ubuntu Builder - Simple Debian Package Builder"
echo "========================================================"

# Package information
PACKAGE_NAME="orangepi-ubuntu-builder"
VERSION="2.0.0-1"
ARCH="amd64"
DEB_FILE="${PACKAGE_NAME}_${VERSION}_${ARCH}.deb"

# Check if we're in the right directory
if [ ! -f "builder.c" ]; then
    echo "Error: Please run this script from the source directory containing builder.c"
    exit 1
fi

# Build the application
echo "Building the application..."
make clean
if ! make all; then
    echo "Error: Build failed"
    exit 1
fi

# Create temporary directory structure
TEMP_DIR=$(mktemp -d)
DEB_DIR="$TEMP_DIR/$PACKAGE_NAME"
echo "Creating package structure in $DEB_DIR"

# Create directory structure
mkdir -p "$DEB_DIR/DEBIAN"
mkdir -p "$DEB_DIR/usr/bin"
mkdir -p "$DEB_DIR/usr/share/doc/$PACKAGE_NAME"
mkdir -p "$DEB_DIR/usr/share/man/man1"
mkdir -p "$DEB_DIR/etc/$PACKAGE_NAME"
mkdir -p "$DEB_DIR/etc/$PACKAGE_NAME/patches"
mkdir -p "$DEB_DIR/etc/$PACKAGE_NAME/patches/uboot"
mkdir -p "$DEB_DIR/etc/$PACKAGE_NAME/patches/kernel"

# Install binary
cp builder "$DEB_DIR/usr/bin/orangepi-ubuntu-builder"
chmod 755 "$DEB_DIR/usr/bin/orangepi-ubuntu-builder"

# Install documentation
cp README.md "$DEB_DIR/usr/share/doc/$PACKAGE_NAME/"
if [ -f "debian/orangepi-ubuntu-builder.1" ]; then
    cp debian/orangepi-ubuntu-builder.1 "$DEB_DIR/usr/share/man/man1/"
    gzip "$DEB_DIR/usr/share/man/man1/orangepi-ubuntu-builder.1" 2>/dev/null || true
fi

# Create configuration file
cat > "$DEB_DIR/etc/$PACKAGE_NAME/builder.conf" << EOF
# Orange Pi Ubuntu Builder Configuration
# Edit this file to customize build parameters
KERNEL_VERSION=6.1
BUILD_JOBS=4
ENABLE_GPU=1
EOF

# Create DEBIAN/control file
cat > "$DEB_DIR/DEBIAN/control" << EOF
Package: $PACKAGE_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: $ARCH
Maintainer: Orange Pi Builder <builder@orangepi.org>
Description: Orange Pi 5 Plus Custom Ubuntu Builder
 A comprehensive tool for building custom Ubuntu images for Orange Pi 5 Plus
 single-board computers. This package provides functionality to:
 .
  * Build custom Linux kernels optimized for Orange Pi 5 Plus
  * Compile and install U-Boot bootloader
  * Create Ubuntu root filesystems with debootstrap
  * Install GPU drivers (Mali G610 with Panfrost/Mesa support)
  * Generate bootable SD card images
  * Manage build dependencies automatically
 .
 The tool supports interactive menu-driven operation and can build complete
 Ubuntu systems from source with hardware-specific optimizations.
Depends: build-essential, gcc-aarch64-linux-gnu, g++-aarch64-linux-gnu, libncurses-dev, gawk, flex, bison, openssl, libssl-dev, git, wget, curl, bc, rsync, python3, debootstrap, fakeroot
EOF

# Create DEBIAN/postinst script
cat > "$DEB_DIR/DEBIAN/postinst" << 'EOF'
#!/bin/bash
set -e

# Create build directories
mkdir -p /tmp/orangepi_build
chmod 755 /tmp/orangepi_build

echo "Orange Pi Ubuntu Builder installed successfully!"
echo "Run 'sudo orangepi-ubuntu-builder' to start building custom Ubuntu images."

exit 0
EOF
chmod 755 "$DEB_DIR/DEBIAN/postinst"

# Create DEBIAN/postrm script
cat > "$DEB_DIR/DEBIAN/postrm" << 'EOF'
#!/bin/bash
set -e

if [ "$1" = "purge" ]; then
    # Remove configuration files on purge
    rm -rf /etc/orangepi-ubuntu-builder
    echo "Orange Pi Ubuntu Builder configuration removed."
fi

exit 0
EOF
chmod 755 "$DEB_DIR/DEBIAN/postrm"

# Calculate installed size (in KB)
INSTALLED_SIZE=$(du -sk "$DEB_DIR" | cut -f1)
echo "Installed-Size: $INSTALLED_SIZE" >> "$DEB_DIR/DEBIAN/control"

# Build the .deb package
echo "Building .deb package..."
dpkg-deb --build "$DEB_DIR" "./$DEB_FILE"

# Cleanup
rm -rf "$TEMP_DIR"

if [ -f "./$DEB_FILE" ]; then
    echo ""
    echo "SUCCESS! Debian package built successfully!"
    echo ""
    echo "Generated file:"
    ls -la "./$DEB_FILE"
    echo ""
    echo "Package info:"
    dpkg --info "./$DEB_FILE"
    echo ""
    echo "To install the package:"
    echo "  sudo dpkg -i $DEB_FILE"
    echo "  sudo apt-get install -f  # Fix any dependency issues"
    echo ""
    echo "To test the package:"
    echo "  sudo orangepi-ubuntu-builder"
else
    echo "Error: Package build failed!"
    exit 1
fi
