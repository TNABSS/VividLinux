#!/bin/bash

echo "=== Local Testing Script for Vivid ==="

# Build the application
echo "🔨 Building application..."
if ! ./build.sh; then
    echo "❌ Build failed"
    exit 1
fi

echo ""
echo "🧪 Running local tests..."

# Test CLI functionality
echo "📋 Testing CLI commands:"
echo "  1. Checking version..."
./builddir/vivid --version

echo "  2. Listing displays..."
./builddir/vivid --list-displays

echo "  3. Checking status..."
./builddir/vivid --status

echo ""
echo "🎮 Testing vibrance control:"

# Get first display
DISPLAY_ID=$(./builddir/vivid --list-displays | grep -o "^\s*[A-Z][A-Z0-9-]*" | head -1 | xargs)

if [ -n "$DISPLAY_ID" ]; then
    echo "  Testing with display: $DISPLAY_ID"
    
    echo "  - Setting vibrance to +25..."
    ./builddir/vivid --display "$DISPLAY_ID" --set-vibrance 25
    sleep 2
    
    echo "  - Setting vibrance to -25..."
    ./builddir/vivid --display "$DISPLAY_ID" --set-vibrance -25
    sleep 2
    
    echo "  - Resetting to normal..."
    ./builddir/vivid --display "$DISPLAY_ID" --reset
    
    echo "✅ CLI tests completed"
else
    echo "⚠️  No displays found for testing"
fi

echo ""
echo "🖥️  Launching GUI for manual testing..."
echo "   - Test the vibrance sliders"
echo "   - Try adding programs"
echo "   - Check the dark theme"
echo "   - Verify display tabs work"
echo ""
echo "Press Ctrl+C to exit GUI and continue..."

# Launch GUI in background and wait
./builddir/vivid &
GUI_PID=$!

# Wait for user to test GUI
read -p "Press Enter when done testing GUI..."

# Kill GUI if still running
if kill -0 $GUI_PID 2>/dev/null; then
    kill $GUI_PID
fi

echo ""
echo "✅ Local testing completed!"
echo ""
echo "📦 Next steps for distribution:"
echo "  1. Test on different AMD systems"
echo "  2. Create Flatpak manifest"
echo "  3. Package for repositories"
echo "  4. Submit to Flathub"
