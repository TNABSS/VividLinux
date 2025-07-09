# 🎮 Vivid - Digital Vibrance Control

**Make your games and videos more colorful on Linux!**

## 🚀 SUPER EASY SETUP (3 steps)

### Step 1: Download
\`\`\`bash
# Download and extract VividLinux-main.zip
# Then open terminal in that folder
\`\`\`

### Step 2: Run the magic script
\`\`\`bash
./EASY-START.sh
\`\`\`
**That's it!** The script does everything automatically:
- ✅ Checks your system
- ✅ Installs what's needed  
- ✅ Builds the app
- ✅ Launches the GUI

### Step 3: Use it!
\`\`\`bash
./run        # Launch GUI anytime
\`\`\`

## 📖 Simple Commands

| Command | What it does |
|---------|-------------|
| `./EASY-START.sh` | First-time setup (does everything) |
| `./run` | Launch the GUI |
| `./install` | Install system-wide |
| `./uninstall` | Remove from system |

## 🎯 What does it do?

- **Adjust screen vibrance** from -100 (gray) to +100 (super colorful)
- **Per-application settings** - different colors for games vs work
- **Multiple monitors** - control each screen separately
- **Works on AMD, NVIDIA, Intel** graphics

## 🖥️ Interface Guide

1. **Display tabs** - Click to switch between monitors
2. **Vibrance slider** - Drag to adjust colors (-100 to +100)
3. **Add program** - Set different colors for specific apps
4. **Focus checkbox** - Only apply when app is active

## ❓ Need Help?

**GUI won't start?**
\`\`\`bash
./builddir/vivid --help    # Check if it works
./builddir/vivid --status  # See what method is used
\`\`\`

**Want to test without GUI?**
\`\`\`bash
./builddir/vivid --list-displays              # Show monitors
./builddir/vivid --display HDMI-0 --set-vibrance 50  # Set vibrance
./builddir/vivid --display HDMI-0 --reset     # Reset to normal
\`\`\`

**Remove everything?**
\`\`\`bash
./uninstall    # Remove from system
rm -rf builddir    # Remove build files
\`\`\`

## 🎮 Perfect for:
- **Gaming** - Make games more vibrant
- **Movies** - Enhance video colors  
- **Photo editing** - Reduce saturation for accurate work
- **Streaming** - Different settings for different content

---
**Made simple for everyone! 🚀**
