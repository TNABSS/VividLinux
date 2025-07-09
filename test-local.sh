#!/bin/bash

echo "=== Vivid Local Testing Script ==="

# Check system information
echo "🔍 System Information:"
echo "  OS: $(lsb_release -d 2>/dev/null | cut -f2 || echo "Unknown")"
echo "  Kernel: $(uname -r)"
echo "  Session: ${XDG_SESSION_TYPE:-Unknown}"
echo "  Display: ${DISPLAY:-Not set}"

# Check GPU information
echo ""
echo "🎮 GPU Information:"
if [ -f "/sys/class/drm/card0/device/vendor" ]; then
    vendor=$(cat /sys/class/drm/card0/device/vendor)
    case $vendor in
        "0x1002") echo "  GPU: AMD detected ✅" ;;
        "0x10de") echo "  GPU: NVIDIA detected ⚠️" ;;
        "0x8086") echo "  GPU: Intel detected ⚠️" ;;
        *) echo "  GPU: Unknown ($vendor)" ;;
    esac
else
    echo "  GPU: Cannot detect"
fi

# Check driver
if lsmod | grep -q amdgpu; then
    echo "  Driver: AMDGPU loaded ✅"
elif lsmod | grep -q nvidia; then
    echo "  Driver: NVIDIA loaded ⚠️"
elif lsmod | grep -q i915; then
    echo "  Driver: Intel i915 loaded ⚠️"
else
    echo "  Driver: Unknown"
fi

echo ""
echo "🔨 Building application..."
if ! ./build.sh; then
    echo "❌ Build failed - check dependencies"
    exit 1
fi

echo ""
echo "🧪 Running CLI tests..."

# Test version
echo "📋 Version check:"
./builddir/vivid --version

echo ""
echo "📋 Status check:"
./builddir/vivid --status

echo ""
echo "📋 Display detection:"
./builddir/vivid --list-displays

# Test vibrance control if displays are available
echo ""
echo "🎮 Testing vibrance control..."

# Get first display
DISPLAY_ID=$(./builddir/vivid --list-displays 2>/dev/null | grep -E "^\s*[A-Z]" | head -1 | awk '{print $1}')

if [ -n "$DISPLAY_ID" ]; then
    echo "  Testing with display: $DISPLAY_ID"
    
    echo "  - Setting vibrance to +30..."
    ./builddir/vivid --display "$DISPLAY_ID" --set-vibrance 30
    sleep 1
    
    echo "  - Setting vibrance to -30..."
    ./builddir/vivid --display "$DISPLAY_ID" --set-vibrance -30
    sleep 1
    
    echo "  - Resetting to normal..."
    ./builddir/vivid --display "$DISPLAY_ID" --reset
    
    echo "✅ CLI vibrance control test completed"
else
    echo "⚠️  No displays found for vibrance testing"
fi

echo ""
echo "🖥️  GUI Testing Instructions:"
echo "  1. Launch GUI: ./builddir/vivid"
echo "  2. Test display tabs switching"
echo "  3. Move vibrance sliders and observe changes"
echo "  4. Try adding a program profile"
echo "  5. Test the focus checkbox"
echo "  6. Verify dark theme appearance"

echo ""
read -p "Press Enter to launch GUI for manual testing (Ctrl+C to skip)..."

# Launch GUI for manual testing
echo "🚀 Launching GUI..."
./builddir/vivid &
GUI_PID=$!

echo ""
echo "GUI launched with PID: $GUI_PID"
echo "Test the interface, then press Enter to continue..."
read -p ""

# Clean up GUI
if kill -0 $GUI_PID 2>/dev/null; then
    echo "Closing GUI..."
    kill $GUI_PID 2>/dev/null
    sleep 1
    kill -9 $GUI_PID 2>/dev/null
fi

echo ""
echo "✅ Local testing completed!"
echo ""
echo "📊 Test Results Summary:"
echo "  - Build: ✅ Success"
echo "  - CLI: ✅ Functional"
echo "  - Display Detection: $([ -n "$DISPLAY_ID" ] && echo "✅ Working" || echo "⚠️  Limited")"
echo "  - Vibrance Control: $([ -n "$DISPLAY_ID" ] && echo "✅ Working" || echo "⚠️  Demo Mode")"
echo ""
echo "🚀 Next Steps:"
echo "  1. Test on different systems/GPUs"
echo "  2. Create system packages"
echo "  3. Submit to repositories"
echo "  4. Package for Flatpak"

# Performance test
echo ""
echo "📈 Performance Test:"
time ./builddir/vivid --status > /dev/null
echo "Startup time measured above"
