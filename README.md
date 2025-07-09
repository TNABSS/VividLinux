# Vivid - Digital Vibrance Control for Linux

A modern, feature-rich digital vibrance control application for AMD GPUs on Linux, providing NVIDIA-like digital vibrance functionality with per-application profiles.

## Features

### Core Functionality
- **Real-time vibrance control** from -100 to +100
- **AMD GPU hardware acceleration** support
- **Multi-display support** with independent control per monitor
- **Per-application profiles** with automatic switching
- **Modern GTK4 dark theme** interface
- **Command-line interface** for scripting and automation

### Advanced Features
- **Automatic program detection** and profile switching
- **Path and title-based matching** for applications
- **Profile management** with save/load functionality
- **System tray integration** (planned)
- **Gaming-focused presets** for popular games

## Installation

### Dependencies

#### Fedora/RHEL/CentOS
\`\`\`bash
# Build dependencies
sudo dnf install gcc-c++ meson ninja-build pkg-config

# Runtime dependencies
sudo dnf install gtk4-devel libX11-devel libXrandr-devel

# Optional: For external monitor control
sudo dnf install ddcutil
\`\`\`

#### Ubuntu/Debian
\`\`\`bash
# Build dependencies
sudo apt install build-essential meson ninja-build pkg-config

# Runtime dependencies
sudo apt install libgtk-4-dev libx11-dev libxrandr-dev

# Optional: For external monitor control
sudo apt install ddcutil
\`\`\`

#### Arch Linux
\`\`\`bash
# Build dependencies
sudo pacman -S base-devel meson ninja pkgconf

# Runtime dependencies
sudo pacman -S gtk4 libx11 libxrandr

# Optional: For external monitor control
sudo pacman -S ddcutil
\`\`\`

### Build from Source

\`\`\`bash
# Clone the repository
git clone https://github.com/your-username/VividLinux.git
cd VividLinux

# Make build script executable
chmod +x build.sh

# Build the application
./build.sh

# Run the application
./builddir/vivid
\`\`\`

### Quick Test
\`\`\`bash
# Make test script executable
chmod +x test-local.sh

# Run comprehensive local tests
./test-local.sh
\`\`\`

## Usage

### GUI Application
Launch the graphical interface:
\`\`\`bash
./builddir/vivid
\`\`\`

**Interface Features:**
- **Display tabs** - Switch between connected monitors
- **Vibrance slider** - Adjust from -100 (desaturated) to +100 (oversaturated)
- **Program list** - Manage per-application profiles
- **Focus checkbox** - Apply vibrance only when specific programs are active

### Command Line Interface
\`\`\`bash
# List all available displays
./builddir/vivid --list-displays

# Set vibrance for a specific display
./builddir/vivid --display DVI-D-0 --set-vibrance 50

# Reset display to normal vibrance
./builddir/vivid --display DVI-D-0 --reset

# Show current status and method
./builddir/vivid --status

# Show help
./builddir/vivid --help
\`\`\`

### Examples
\`\`\`bash
# Increase vibrance for gaming
./builddir/vivid --display HDMI-0 --set-vibrance 75

# Reduce vibrance for photo editing
./builddir/vivid --display DP-1 --set-vibrance -25

# Reset all displays
for display in $(./builddir/vivid --list-displays | grep -o "^\s*[A-Z][A-Z0-9-]*"); do
    ./builddir/vivid --display "$display" --reset
done
\`\`\`

## Supported Hardware

### Graphics Cards
- **AMD GPUs** with AMDGPU driver (primary support)
- **NVIDIA GPUs** (limited support via xrandr)
- **Intel GPUs** (basic support via xrandr)

### Display Connections
- **Internal displays** (laptop screens, eDP)
- **External monitors** via HDMI, DisplayPort, DVI
- **Multiple displays** with independent control

### Methods Used
1. **AMD Color Properties** - Hardware-level control (best quality)
2. **XRandR CTM** - Color transformation matrix (good compatibility)
3. **DDC/CI** - Direct monitor communication (external displays)
4. **Demo Mode** - For testing without hardware support

## System Requirements

### Minimum Requirements
- **OS**: Linux with X11 or Wayland
- **GPU**: Any graphics card with driver support
- **RAM**: 50MB
- **Storage**: 10MB

### Recommended Requirements
- **OS**: Fedora 39+, Ubuntu 22.04+, or Arch Linux
- **GPU**: AMD GPU with AMDGPU driver
- **Session**: X11 (for full functionality)
- **Permissions**: User in `video` group for hardware access

## Troubleshooting

### Common Issues

**"No displays found"**
- Ensure you're running on X11: `echo $XDG_SESSION_TYPE`
- Check if displays are connected: `xrandr --listmonitors`

**"Build failed"**
- Install all build dependencies listed above
- Check GCC version: `gcc --version` (requires GCC 7+)

**"Permission denied for hardware control"**
- Add user to video group: `sudo usermod -a -G video $USER`
- Log out and back in for changes to take effect

**"Vibrance not working"**
- Check current method: `./builddir/vivid --status`
- For AMD GPUs, ensure AMDGPU driver is loaded: `lsmod | grep amdgpu`

### Debug Mode
\`\`\`bash
# Enable verbose output
export VIVID_DEBUG=1
./builddir/vivid --status
\`\`\`

### Reset Configuration
\`\`\`bash
# Remove config files
rm -rf ~/.config/vivid/
\`\`\`

## Development

### Building for Development
\`\`\`bash
# Configure with debug symbols
meson setup builddir --buildtype=debug

# Compile with verbose output
meson compile -C builddir -v

# Run with debugging
gdb ./builddir/vivid
\`\`\`

### Contributing
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## Packaging

### Flatpak
\`\`\`bash
# Build Flatpak package
flatpak-builder build-dir flatpak/org.vivid.SaturationControl.yml --force-clean

# Install locally
flatpak-builder --user --install build-dir flatpak/org.vivid.SaturationControl.yml --force-clean
\`\`\`

### System Package
\`\`\`bash
# Install system-wide
sudo meson install -C builddir

# Create package (example for RPM)
rpmbuild -ba vivid.spec
\`\`\`

## License

GPL-3.0+ - See [LICENSE](LICENSE) for details.

## Roadmap

- [ ] **Wayland native support** with color-management-v1 protocol
- [ ] **KDE integration** with KWin effects
- [ ] **GNOME Shell extension** for quick access
- [ ] **Automatic game detection** via Steam/Lutris integration
- [ ] **HDR support** when protocols mature
- [ ] **System tray** with quick presets
- [ ] **Web interface** for remote control

## Support

- **Issues**: [GitHub Issues](https://github.com/your-username/VividLinux/issues)
- **Discussions**: [GitHub Discussions](https://github.com/your-username/VividLinux/discussions)
- **Matrix**: `#vivid:matrix.org`

---

**Made with ❤️ for the Linux gaming and creative community**
