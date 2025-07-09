#!/bin/bash

echo "🎮 Testing Real Vibrance Control"
echo "================================"

if [ ! -f "builddir/vivid" ]; then
    echo "❌ Please build first: ./quick-build.sh"
    exit 1
fi

echo "📺 Available displays:"
./builddir/vivid --list

echo ""
echo "🧪 Testing vibrance changes..."

# Get first display
DISPLAY_ID=$(./builddir/vivid --list 2>/dev/null | grep -E "^\s*[A-Z]" | head -1 | awk '{print $1}')

if [ -n "$DISPLAY_ID" ]; then
    echo "🎯 Testing with display: $DISPLAY_ID"
    
    echo "  Setting vibrance to +50..."
    ./builddir/vivid --set "$DISPLAY_ID" 50
    sleep 2
    
    echo "  Setting vibrance to -50..."
    ./builddir/vivid --set "$DISPLAY_ID" -50
    sleep 2
    
    echo "  Resetting to normal..."
    ./builddir/vivid --reset "$DISPLAY_ID"
    
    echo "✅ Vibrance test completed!"
    echo "   Did you see your screen colors change?"
else
    echo "❌ No displays found for testing"
fi

echo ""
echo "🚀 Launch GUI for interactive testing:"
echo "   ./builddir/vivid"
