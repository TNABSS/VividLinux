#!/bin/bash

echo "⚡ Quick Test Script"
echo "==================="

# Build if needed
if [ ! -f "builddir/vivid" ]; then
    echo "Building..."
    ./build.sh || exit 1
fi

echo ""
echo "🧪 Testing CLI:"
./builddir/vivid --status
echo ""
./builddir/vivid --list-displays

echo ""
echo "🖥️  Launching GUI..."
./builddir/vivid
