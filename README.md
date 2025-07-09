USAGE:
  ./vivid                    - Setup and launch GUI
  ./vivid build             - Build only
  ./vivid gui               - Launch GUI (build if needed)
  ./vivid install-deps      - Install dependencies only
  ./vivid clean             - Clean build files
  ./vivid install           - Install system-wide
  ./vivid fix               - Fix permissions
  ./vivid help              - Show this help

CLI COMMANDS (after building):
  ./builddir/vivid --list                    - List displays
  ./builddir/vivid --set <display> <value>   - Set vibrance
  ./builddir/vivid --reset-all               - Reset all

EXAMPLES:
  ./vivid                                    - Full setup & launch
  ./builddir/vivid --set HDMI-A-1 50        - Set HDMI to +50
  ./builddir/vivid --reset-all               - Reset everything




This is just a working demo, feel free to reach out and give support on this project, currently the project is almost out of the alpha stage and is launchable, however the saturation changing aspect in xrandr is quite broken and buggy, im working on it.
This is still my 1st project to give support, under the Gnu 3.0 Feel free to use the code and make it better, with care --METADev.
