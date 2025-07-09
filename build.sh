#!/bin/bash

echo "=== Vivid Digital Vibrance Control Build Script ==="

# Check if we're on a supported system
if ! command -v meson &> /dev/null; then
    echo "❌ Meson not found. Install with:"
    echo "   Fedora: sudo dnf install meson ninja-build"
    echo "   Ubuntu: sudo apt install meson ninja-build"
    exit 1
fi

# Check for required dependencies
echo "🔍 Checking dependencies..."

if ! pkg-config --exists gtk4; then
    echo "❌ GTK4 not found. Install with:"
    echo "   Fedora: sudo dnf install gtk4-devel"
    echo "   Ubuntu: sudo apt install libgtk-4-dev"
    exit 1
fi

if pkg-config --exists x11 xrandr; then
    echo "✅ X11 support available"
else
    echo "⚠️  X11 support not available (install libx11-devel libxrandr-devel)"
fi

# Check for AMD GPU
if [ -f "/sys/class/drm/card0/device/vendor" ]; then
    vendor=$(cat /sys/class/drm/card0/device/vendor)
    if [ "$vendor" = "0x1002" ]; then
        echo "✅ AMD GPU detected"
    else
        echo "⚠️  Non-AMD GPU detected - some features may not work"
    fi
fi

# Clean previous build
if [ -d "builddir" ]; then
    echo "🧹 Cleaning previous build..."
    rm -rf builddir
fi

# Configure build
echo "⚙️  Configuring build..."
if ! meson setup builddir; then
    echo "❌ Meson setup failed"
    exit 1
fi

# Compile
echo "🔨 Compiling..."
if ! meson compile -C builddir; then
    echo "❌ Compilation failed"
    exit 1
fi

echo "✅ Build successful!"
echo ""
echo "🚀 Usage:"
echo "  ./builddir/vivid                                    # Launch GUI"
echo "  ./builddir/vivid --list-displays                    # List displays"
echo "  ./builddir/vivid --display DVI-D-0 --set-vibrance 50  # Set vibrance"
echo "  ./builddir/vivid --status                           # Show status"
echo ""
echo "📦 To install system-wide:"
echo "  sudo meson install -C builddir"
echo ""
echo "🔧 For development:"
echo "  meson configure builddir -Dprefix=/usr/local"
