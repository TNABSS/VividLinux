```
USAGE:
  ./vivid             - Setup and launch GUI
  ./vivid build       - Build only
  ./vivid gui         - Launch GUI (build if needed)
  ./vivid install-deps  - Install dependencies only
  ./vivid clean       - Clean build files
  ./vivid install     - Install system-wide
  ./vivid fix         - Fix permissions
  ./vivid help        - Show this help

CLI COMMANDS (after building):
  ./builddir/vivid --list               - List displays
  ./builddir/vivid --set <display> <value>  - Set vibrance
  ./builddir/vivid --reset-all          - Reset all

EXAMPLES:
  ./vivid                               - Full setup & launch
  ./builddir/vivid --set HDMI-A-1 50    - Set HDMI to +50
  ./builddir/vivid --reset-all          - Reset everything
```

This is a working demo of the Vivid project. We welcome your feedback and contributions to help us improve it.

Currently, the project is nearing the end of its alpha stage and is in a launchable state. However, please be aware that the saturation adjustment feature, which relies on `xrandr`, is currently experiencing some known bugs and instability. We are actively working on resolving these issues.

This is our first open-source project, released under the GNU General Public License v3.0. Feel free to use, modify, and distribute the code, with the hope that you'll contribute to making it even better.

-- METADev
