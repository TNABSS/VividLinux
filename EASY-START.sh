#!/bin/bash

# 🚀 VIVID - SUPER EASY STARTER SCRIPT
# ===================================
# This script does EVERYTHING for you!

set -e  # Exit on any error

echo "🎮 VIVID - Digital Vibrance Control"
echo "==================================="
echo ""
echo "This script will:"
echo "  ✅ Check your system"
echo "  ✅ Install missing dependencies"
echo "  ✅ Build the application"
echo "  ✅ Launch the GUI"
echo ""

# Function to detect OS
detect_os() {
    if [ -f /etc/fedora-release ]; then
        echo "fedora"
    elif [ -f /etc/debian_version ]; then
        echo "debian"
    elif [ -f /etc/arch-release ]; then
        echo "arch"
    else
        echo "unknown"
    fi
}

# Function to install dependencies
install_deps() {
    local os=$1
    echo "📦 Installing dependencies for $os..."
    
    case $os in
        "fedora")
            sudo dnf install -y gcc-c++ meson ninja-build pkg-config gtk4-devel libX11-devel libXrandr-devel
            ;;
        "debian")
            sudo apt update
            sudo apt install -y build-essential meson ninja-build pkg-config libgtk-4-dev libx11-dev libxrandr-dev
            ;;
        "arch")
            sudo pacman -S --needed base-devel meson ninja pkgconf gtk4 libx11 libxrandr
            ;;
        *)
            echo "❌ Unsupported OS. Please install manually:"
            echo "   - gcc/g++ compiler"
            echo "   - meson build system"
            echo "   - ninja build tool"
            echo "   - GTK4 development files"
            echo "   - X11 development files"
            exit 1
            ;;
    esac
}

# Check if we're in the right directory
if [ ! -f "meson.build" ]; then
    echo "❌ ERROR: Please run this script from the VividLinux directory!"
    echo ""
    echo "💡 How to fix:"
    echo "   1. Download/extract VividLinux"
    echo "   2. cd VividLinux-main"
    echo "   3. ./EASY-START.sh"
    echo ""
    exit 1
fi

# Detect OS
OS=$(detect_os)
echo "🖥️  Detected OS: $OS"

# Check for dependencies
echo ""
echo "🔍 Checking dependencies..."

MISSING_DEPS=()

# Check for compiler
if ! command -v g++ &> /dev/null; then
    MISSING_DEPS+=("compiler")
fi

# Check for meson
if ! command -v meson &> /dev/null; then
    MISSING_DEPS+=("meson")
fi

# Check for ninja
if ! command -v ninja &> /dev/null; then
    MISSING_DEPS+=("ninja")
fi

# Check for pkg-config
if ! command -v pkg-config &> /dev/null; then
    MISSING_DEPS+=("pkg-config")
fi

# Check for GTK4
if ! pkg-config --exists gtk4; then
    MISSING_DEPS+=("gtk4")
fi

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    echo "⚠️  Missing dependencies: ${MISSING_DEPS[*]}"
    echo ""
    read -p "🤔 Install missing dependencies automatically? (y/N): " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_deps $OS
        echo "✅ Dependencies installed!"
    else
        echo "❌ Cannot continue without dependencies"
        exit 1
    fi
else
    echo "✅ All dependencies found!"
fi

# Show system info
echo ""
echo "💻 System Information:"
echo "   Session: ${XDG_SESSION_TYPE:-Unknown}"
echo "   Desktop: ${XDG_CURRENT_DESKTOP:-Unknown}"

if [ -f "/sys/class/drm/card0/device/vendor" ]; then
    vendor=$(cat /sys/class/drm/card0/device/vendor 2>/dev/null || echo "unknown")
    case $vendor in
        "0x1002") echo "   GPU: AMD ✅ (Perfect!)" ;;
        "0x10de") echo "   GPU: NVIDIA ⚠️ (Limited support)" ;;
        "0x8086") echo "   GPU: Intel ⚠️ (Basic support)" ;;
        *) echo "   GPU: Unknown" ;;
    esac
else
    echo "   GPU: Cannot detect"
fi

# Clean any previous builds
echo ""
echo "🧹 Cleaning previous builds..."
rm -rf builddir

# Build the application
echo ""
echo "🔨 Building Vivid..."
echo "   (This may take 1-2 minutes)"

if meson setup builddir --buildtype=release; then
    echo "✅ Configuration successful"
else
    echo "❌ Configuration failed"
    exit 1
fi

if meson compile -C builddir; then
    echo "✅ Build successful!"
else
    echo "❌ Build failed"
    exit 1
fi

# Test the application
echo ""
echo "🧪 Testing application..."
if ./builddir/vivid --version > /dev/null 2>&1; then
    echo "✅ Application works!"
else
    echo "❌ Application test failed"
    exit 1
fi

# Show usage instructions
echo ""
echo "🎉 SUCCESS! Vivid is ready to use!"
echo ""
echo "📖 How to use:"
echo "   GUI:  ./builddir/vivid"
echo "   Help: ./builddir/vivid --help"
echo "   Test: ./builddir/vivid --status"
echo ""

# Ask if user wants to launch GUI
read -p "🚀 Launch the GUI now? (Y/n): " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Nn]$ ]]; then
    echo "🖥️  Launching Vivid GUI..."
    echo "   (Close the window or press Ctrl+C to exit)"
    echo ""
    exec ./builddir/vivid
fi

echo ""
echo "✅ Setup complete! Run './builddir/vivid' anytime to use Vivid."
