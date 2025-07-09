#!/bin/bash

echo "=== Vivid Build Script ==="

# Check if we're on a supported system
if ! command -v meson &> /dev/null; then
    echo "❌ Meson not found. Install with:"
    echo "   Fedora: sudo dnf install meson"
    echo "   Ubuntu: sudo apt install meson"
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
echo "Usage:"
echo "  ./builddir/vivid                                    # Launch GUI"
echo "  ./builddir/vivid --list-displays                    # List displays"
echo "  ./builddir/vivid --display DP-1 --set-saturation 150  # Set saturation"
echo "  ./builddir/vivid --status                           # Show status"
