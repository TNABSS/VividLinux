# Vivid - Digital Vibrance Control

Simple digital vibrance control for Linux displays.

## Installation

\`\`\`bash
./vivid
\`\`\`

That's it. The script will install dependencies and launch the GUI.

## Usage

### GUI
- Launch: `./vivid`
- Move sliders to adjust vibrance (-100 to 100)
- Click Reset to restore normal colors
- Click Install for system-wide installation

### Command Line
\`\`\`bash
vivid --list                    # List displays
vivid --set HDMI-A-1 50        # Set display vibrance
vivid --reset                  # Reset all displays
\`\`\`

## Requirements

- Linux with X11
- xrandr (usually pre-installed)
- GTK4 (auto-installed)

## How It Works

Uses multiple methods for maximum compatibility:
- xgamma for color adjustment
- redshift for color temperature
- xcalib for ICC profiles
- xrandr as fallback

Changes are applied immediately and persist until reset.
\`\`\`

```shellscript file="vivid"
#!/bin/bash

set -e

# Simple launcher - no verbose output
if [ ! -f "meson.build" ]; then
    echo "Error: Run from VividLinux directory"
    exit 1
fi

# Fix permissions silently
chmod +x "$0" 2>/dev/null || true
find . -maxdepth 1 -name "*.sh" -exec chmod +x {} \; 2>/dev/null || true

# Install dependencies if missing
if ! command -v meson >/dev/null || ! pkg-config --exists gtk4; then
    if [ -f /etc/fedora-release ]; then
        sudo dnf install -y gcc-c++ meson ninja-build pkg-config gtk4-devel libX11-devel libXrandr-devel xrandr >/dev/null 2>&1
    elif [ -f /etc/debian_version ]; then
        sudo apt update >/dev/null 2>&1
        sudo apt install -y build-essential meson ninja-build pkg-config libgtk-4-dev libx11-dev libxrandr-dev x11-xserver-utils >/dev/null 2>&1
    elif [ -f /etc/arch-release ]; then
        sudo pacman -S --noconfirm base-devel meson ninja pkgconf gtk4 libx11 libxrandr xorg-xrandr >/dev/null 2>&1
    fi
fi

# Build if needed
if [ ! -f "builddir/vivid" ]; then
    rm -rf builddir >/dev/null 2>&1
    meson setup builddir --buildtype=release >/dev/null 2>&1
    meson compile -C builddir >/dev/null 2>&1
fi

# Handle commands
case "${1:-}" in
    "clean")
        rm -rf builddir
        ;;
    "install")
        if [ -f "builddir/vivid" ]; then
            pkexec cp builddir/vivid /usr/local/bin/vivid 2>/dev/null || sudo cp builddir/vivid /usr/local/bin/vivid
        fi
        ;;
    *)
        if [ -f "builddir/vivid" ]; then
            exec ./builddir/vivid "$@"
        fi
        ;;
esac
