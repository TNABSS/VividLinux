# 🚀 Vivid Autostart Feature Guide

## Overview

The autostart feature allows Vivid to automatically start when you log in and apply your saved vibrance settings. This ensures consistent colors across reboots without manual intervention.

## Features

### ✅ What It Does
- **Automatic startup** - Vivid starts when you log in
- **Profile application** - Saved vibrance settings are applied automatically
- **Delayed start** - Configurable delay to avoid conflicts with other startup programs
- **Safe operation** - Extensive validation and error checking
- **Cross-desktop support** - Works with GNOME, KDE, XFCE, and other desktop environments

### 🛡️ Safety Features
- **Desktop file validation** - Ensures the autostart file is correctly formatted
- **Permission checking** - Verifies proper file permissions
- **Executable verification** - Confirms the Vivid executable exists
- **Graceful fallbacks** - Handles missing directories and permissions issues
- **Debug information** - Comprehensive troubleshooting tools

## How to Use

### Enable Autostart
1. Open Vivid GUI
2. Scroll to "🚀 Autostart Configuration" section
3. Check "Start Vivid automatically when I log in"
4. Configure options as desired
5. Click OK when prompted

### Disable Autostart
1. Open Vivid GUI
2. Uncheck the autostart checkbox
3. Confirm when prompted

### Debug Issues
1. Click "🔧 Debug Autostart" button
2. Review the detailed information
3. Copy debug info if needed for support

## Configuration Options

### Startup Behavior
- **Start minimized** - Start in system tray (when implemented)
- **Apply profiles** - Automatically apply saved vibrance settings
- **Startup delay** - Wait 0-30 seconds before starting (default: 3 seconds)

### Command Line Options
When started via autostart, Vivid supports these arguments:
- `--minimize` - Start minimized to tray
- `--apply-profiles` - Apply saved profiles on startup
- `--delay N` - Wait N seconds before starting

## Technical Details

### Desktop File Location
- **Path**: `~/.config/autostart/org.vivid.SaturationControl.desktop`
- **Format**: Standard XDG Desktop Entry
- **Permissions**: 644 (readable by user and group)

### Supported Desktop Environments
- **GNOME** - Full support with session management
- **KDE Plasma** - Full support with startup notifications
- **XFCE** - Full support
- **MATE** - Full support
- **Cinnamon** - Full support
- **Others** - Basic support via XDG autostart

### Environment Variables Used
- `XDG_CONFIG_HOME` - Custom config directory (if set)
- `HOME` - User home directory
- `XDG_CURRENT_DESKTOP` - Desktop environment detection
- `XDG_SESSION_TYPE` - Session type (X11/Wayland)

## Troubleshooting

### Common Issues

**Autostart doesn't work**
1. Check if `~/.config/autostart` directory exists
2. Verify desktop file permissions
3. Use debug feature to check configuration
4. Ensure Vivid executable path is correct

**Desktop file validation fails**
1. Check file format with debug feature
2. Verify executable path exists
3. Ensure proper permissions on autostart directory

**Profiles not applied on startup**
1. Verify profiles are saved correctly
2. Check "Apply profiles" option is enabled
3. Increase startup delay if needed

### Debug Information

The debug feature provides:
- **File system status** - Directory and file existence
- **Permission information** - File and directory permissions
- **Content validation** - Desktop file format checking
- **Environment details** - System configuration
- **Error messages** - Specific failure reasons

### Manual Testing

Test autostart manually:
\`\`\`bash
# Check current status
./builddir/vivid --status

# Test autostart file
desktop-file-validate ~/.config/autostart/org.vivid.SaturationControl.desktop

# Simulate autostart
./builddir/vivid --apply-profiles --delay 3
\`\`\`

## Advanced Configuration

### Custom Desktop File
You can manually edit the desktop file for advanced options:
\`\`\`ini
[Desktop Entry]
Type=Application
Name=Vivid Digital Vibrance Control
Exec=/path/to/vivid --apply-profiles --delay 5
Hidden=false
X-GNOME-Autostart-enabled=true
X-GNOME-Autostart-Delay=5
\`\`\`

### System-wide Installation
For system-wide autostart (all users):
1. Install Vivid system-wide: `sudo ./install`
2. Copy desktop file to `/etc/xdg/autostart/`
3. Modify Exec path to system location

## Security Considerations

### Safe Practices
- **Limited permissions** - Desktop file has minimal required permissions
- **Path validation** - Executable paths are verified
- **User-only access** - Autostart files are user-specific
- **No elevated privileges** - Runs with normal user permissions

### What's Protected
- **System integrity** - No system-wide changes
- **User privacy** - Configuration stays in user directory
- **Resource usage** - Startup delay prevents resource conflicts
- **Error handling** - Graceful failure without system impact

---

**The autostart feature is designed to be safe, reliable, and easy to use while providing comprehensive debugging tools for troubleshooting.**
