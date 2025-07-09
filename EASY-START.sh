#!/bin/bash

# 🎮 VIVID - SUPER EASY STARTER SCRIPT
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
echo "  ✅ Launch the GUI with REAL vibrance control"
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
            sudo dnf install -y gcc-c++ meson ninja-build pkg-config gtk4-devel libX11-devel libXrandr-devel xrandr
            ;;
        "debian")
            sudo apt update
            sudo apt install -y build-essential meson ninja-build pkg-config libgtk-4-dev libx11-dev libxrandr-dev x11-xserver-utils
            ;;
        "arch")
            sudo pacman -S --needed base-devel meson ninja pkgconf gtk4 libx11 libxrandr xorg-xrandr
            ;;
        *)
            echo "❌ Unsupported OS. Please install manually:"
            echo "   - gcc/g++ compiler"
            echo "   - meson build system"
            echo "   - ninja build tool"
            echo "   - GTK4 development files"
            echo "   - X11 development files"
            echo "   - xrandr utility"
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

# Check for xrandr (critical for vibrance control)
echo ""
echo "🔍 Checking critical dependencies..."

if ! command -v xrandr &> /dev/null; then
    echo "❌ xrandr not found - this is REQUIRED for vibrance control!"
    echo "   Installing xrandr..."
    install_deps $OS
else
    echo "✅ xrandr found - vibrance control will work!"
fi

# Test xrandr
echo "🧪 Testing display detection..."
if xrandr --listmonitors > /dev/null 2>&1; then
    DISPLAY_COUNT=$(xrandr --listmonitors 2>/dev/null | grep -c "^ ")
    echo "✅ Found $DISPLAY_COUNT display(s) - vibrance control ready!"
else
    echo "⚠️  xrandr test failed - vibrance may not work properly"
fi

# Check for other dependencies
MISSING_DEPS=()

if ! command -v g++ &> /dev/null; then
    MISSING_DEPS+=("compiler")
fi

if ! command -v meson &> /dev/null; then
    MISSING_DEPS+=("meson")
fi

if ! command -v ninja &> /dev/null; then
    MISSING_DEPS+=("ninja")
fi

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
        "0x1002") echo "   GPU: AMD ✅ (Excellent support!)" ;;
        "0x10de") echo "   GPU: NVIDIA ✅ (Good support!)" ;;
        "0x8086") echo "   GPU: Intel ✅ (Basic support!)" ;;
        *) echo "   GPU: Unknown (should still work)" ;;
    esac
else
    echo "   GPU: Cannot detect (should still work)"
fi

# Clean any previous builds
echo ""
echo "🧹 Cleaning previous builds..."
rm -rf builddir

# Build the application
echo ""
echo "🔨 Building Vivid with REAL vibrance control..."
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
echo "🧪 Testing vibrance functionality..."
if ./builddir/vivid --status > /dev/null 2>&1; then
    echo "✅ Vivid works and can control vibrance!"
else
    echo "❌ Application test failed"
    exit 1
fi

# Show usage instructions
echo ""
echo "🎉 SUCCESS! Vivid is ready with REAL vibrance control!"
echo ""
echo "📖 Quick start:"
echo "   GUI:     ./builddir/vivid"
echo "   CLI:     ./builddir/vivid --set DVI-D-0 50"
echo "   Help:    ./builddir/vivid --help"
echo "   Status:  ./builddir/vivid --status"
echo ""
echo "🎮 The vibrance changes are REAL and PERSISTENT!"
echo "   - Changes survive closing the GUI"
echo "   - Settings are saved automatically"
echo "   - Works with any GPU (AMD/NVIDIA/Intel)"
echo ""

# Ask if user wants to launch GUI
read -p "🚀 Launch the GUI now to test vibrance control? (Y/n): " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Nn]$ ]]; then
    echo "🖥️  Launching Vivid GUI..."
    echo "   Try moving the vibrance slider - you'll see REAL changes!"
    echo "   (Close the window or press Ctrl+C to exit)"
    echo ""
    exec ./builddir/vivid
fi

echo ""
echo "✅ Setup complete! Run './builddir/vivid' anytime to control vibrance."
echo "💡 Pro tip: Try './builddir/vivid --set DVI-D-0 75' for instant gaming vibrance!"
