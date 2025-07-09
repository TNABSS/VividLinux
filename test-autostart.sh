#!/bin/bash

echo "🧪 Testing Vivid Autostart Feature"
echo "=================================="

# Check if built
if [ ! -f "builddir/vivid" ]; then
    echo "❌ Please build first with ./EASY-START.sh"
    exit 1
fi

echo ""
echo "🔍 Current autostart status:"
./builddir/vivid --status | grep -i autostart || echo "  No autostart info in status"

echo ""
echo "📁 Checking autostart directory:"
AUTOSTART_DIR="$HOME/.config/autostart"
echo "  Directory: $AUTOSTART_DIR"
echo "  Exists: $([ -d "$AUTOSTART_DIR" ] && echo "Yes" || echo "No")"

if [ -d "$AUTOSTART_DIR" ]; then
    echo "  Contents:"
    ls -la "$AUTOSTART_DIR" | grep -E "(vivid|Vivid)" || echo "    No Vivid autostart files found"
fi

echo ""
echo "🧪 Testing autostart enable/disable:"
echo "  (This will be done through the GUI)"

echo ""
echo "🚀 Launch GUI to test autostart feature:"
echo "  1. Open Vivid GUI"
echo "  2. Scroll to 'Autostart Configuration' section"
echo "  3. Toggle the autostart checkbox"
echo "  4. Click 'Debug Autostart' for detailed info"
echo "  5. Check if desktop file is created in ~/.config/autostart/"

echo ""
read -p "Press Enter to launch GUI for autostart testing..."

./builddir/vivid
