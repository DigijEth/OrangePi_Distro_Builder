#!/bin/bash

# Orange Pi Ubuntu Builder - Debian Package Build Script
# This script builds a .deb package from the source code

set -e

echo "Orange Pi Ubuntu Builder - Debian Package Builder"
echo "=================================================="

# Check if we're in the right directory
if [ ! -f "builder.c" ] || [ ! -d "debian" ]; then
    echo "Error: Please run this script from the source directory containing builder.c and debian/ folder"
    exit 1
fi

# Check if required tools are installed
check_tool() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 is not installed. Please install it first:"
        echo "  sudo apt-get install $2"
        exit 1
    fi
}

echo "Checking required tools..."
check_tool "dpkg-buildpackage" "dpkg-dev"
check_tool "dh" "debhelper"
check_tool "gcc" "build-essential"

# Make sure debian/rules is executable
chmod +x debian/rules

# Clean any previous builds
echo "Cleaning previous builds..."
make clean 2>/dev/null || true
rm -f ../orangepi-ubuntu-builder_*.deb 2>/dev/null || true
rm -f ../orangepi-ubuntu-builder_*.changes 2>/dev/null || true
rm -f ../orangepi-ubuntu-builder_*.buildinfo 2>/dev/null || true

# First, try to compile to make sure everything works
echo "Testing compilation..."
if ! make all; then
    echo "Error: Compilation failed. Please fix the code first."
    exit 1
fi

echo "Compilation successful! Proceeding with package build..."
make clean

# Build the Debian package
echo "Building Debian package..."
dpkg-buildpackage -us -uc -b

if [ $? -eq 0 ]; then
    echo ""
    echo "SUCCESS! Debian package built successfully!"
    echo ""
    echo "Generated files:"
    ls -la ../orangepi-ubuntu-builder_*.deb 2>/dev/null || echo "No .deb file found"
    ls -la ../orangepi-ubuntu-builder_*.changes 2>/dev/null || echo "No .changes file found"
    echo ""
    echo "To install the package:"
    echo "  sudo dpkg -i ../orangepi-ubuntu-builder_*.deb"
    echo "  sudo apt-get install -f  # Fix any dependency issues"
    echo ""
    echo "To test the package:"
    echo "  sudo orangepi-ubuntu-builder"
else
    echo "Error: Package build failed!"
    exit 1
fi
