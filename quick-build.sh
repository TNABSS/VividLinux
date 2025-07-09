#!/bin/bash

echo "🔨 Quick Build & Test"
echo "===================="

# Clean and rebuild
rm -rf builddir
echo "🧹 Cleaned build directory"

# Configure
if meson setup builddir --buildtype=release; then
    echo "✅ Configuration successful"
else
    echo "❌ Configuration failed"
    exit 1
fi

# Build
if meson compile -C builddir; then
    echo "✅ Build successful!"
else
    echo "❌ Build failed"
    exit 1
fi

# Test CLI
echo ""
echo "🧪 Testing CLI functionality..."
./builddir/vivid --help

echo ""
echo "📺 Detecting displays..."
./builddir/vivid --list

echo ""
echo "✅ Build complete! Ready to use:"
echo "   GUI: ./builddir/vivid"
echo "   CLI: ./builddir/vivid --set DVI-D-0 50"
