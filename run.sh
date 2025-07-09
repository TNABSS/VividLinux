#!/bin/bash

echo "🚀 Vivid - Easy Launcher"
echo "========================"

# Check if we're in the right directory
if [ ! -f "meson.build" ]; then
    echo "❌ Please run this from the VividLinux-main directory"
    echo "   cd ~/Lejupielādes/VividLinux-main"
    echo "   ./run.sh"
    exit 1
fi

# Build if needed
if [ ! -f "builddir/vivid" ]; then
    echo "🔨 Building application..."
    if ! ./build.sh; then
        echo "❌ Build failed"
        exit 1
    fi
else
    echo "✅ Application already built"
fi

# Show system info
echo ""
echo "💻 System Information:"
echo "   Session: ${XDG_SESSION_TYPE:-Unknown}"
echo "   Desktop: ${XDG_CURRENT_DESKTOP:-Unknown}"
if [ -f "/sys/class/drm/card0/device/vendor" ]; then
    vendor=$(cat /sys/class/drm/card0/device/vendor)
    case $vendor in
        "0x1002") echo "   GPU: AMD ✅" ;;
        "0x10de") echo "   GPU: NVIDIA" ;;
        "0x8086") echo "   GPU: Intel" ;;
        *) echo "   GPU: Unknown" ;;
    esac
fi

echo ""
echo "🎮 Launching Vivid GUI..."
echo "   (The interface will work even in demo mode!)"
echo ""

# Launch the application
exec ./builddir/vivid
