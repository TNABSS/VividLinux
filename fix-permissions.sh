#!/bin/bash

echo "🔧 Fixing ALL permission issues..."

# Make everything executable
chmod +x vivid 2>/dev/null || true
chmod +x *.sh 2>/dev/null || true
chmod +x run 2>/dev/null || true
chmod +x v 2>/dev/null || true
chmod +x install 2>/dev/null || true
chmod +x uninstall 2>/dev/null || true

# Make this script executable too
chmod +x "$0" 2>/dev/null || true

echo "✅ All permissions fixed!"
echo ""
echo "Now you can run:"
echo "  ./vivid           - Setup and launch"
echo "  ./vivid gui       - Quick launch"
echo "  ./vivid build     - Build only"
