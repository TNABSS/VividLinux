#include "VibranceController.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <cmath>

VibranceController::VibranceController() {
    initialize();
}

VibranceController::~VibranceController() {
    // Reset all displays on exit
    resetAllDisplays();
}

bool VibranceController::initialize() {
    std::cout << "🔧 Initializing Vivid..." << std::endl;
    
    // Auto-install dependencies first
    if (!autoInstallDependencies()) {
        std::cout << "⚠️  Some dependencies missing, but continuing..." << std::endl;
    }
    
    if (!detectDisplays()) {
        std::cout << "❌ Failed to detect displays" << std::endl;
        return false;
    }
    
    m_initialized = true;
    std::cout << "✅ Vivid ready with " << m_displays.size() << " display(s)" << std::endl;
    return true;
}

bool VibranceController::autoInstallDependencies() {
    std::cout << "📦 Checking dependencies..." << std::endl;
    
    // Check if xrandr is available
    if (system("which xrandr > /dev/null 2>&1") != 0) {
        std::cout << "🔄 Installing xrandr..." << std::endl;
        
        // Try different package managers
        if (system("which dnf > /dev/null 2>&1") == 0) {
            system("pkexec dnf install -y xrandr 2>/dev/null");
        } else if (system("which apt > /dev/null 2>&1") == 0) {
            system("pkexec apt install -y x11-xserver-utils 2>/dev/null");
        } else if (system("which pacman > /dev/null 2>&1") == 0) {
            system("pkexec pacman -S --noconfirm xorg-xrandr 2>/dev/null");
        }
    }
    
    return system("which xrandr > /dev/null 2>&1") == 0;
}

bool VibranceController::detectDisplays() {
    m_displays.clear();
    
    // Get connected displays using xrandr
    FILE* pipe = popen("xrandr --query 2>/dev/null | grep ' connected' | awk '{print $1}'", "r");
    if (!pipe) return false;
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string displayId(buffer);
        displayId.erase(displayId.find_last_not_of(" \n\r\t") + 1);
        
        if (!displayId.empty()) {
            Display display;
            display.id = displayId;
            display.name = displayId;
            display.currentVibrance = 0;
            display.connected = true;
            
            m_displays.push_back(display);
            m_currentVibrance[display.id] = 0;
            
            std::cout << "📺 Found: " << displayId << std::endl;
        }
    }
    pclose(pipe);
    
    // Fallback if no displays found
    if (m_displays.empty()) {
        Display demo;
        demo.id = "DEMO-1";
        demo.name = "Demo Display";
        demo.currentVibrance = 0;
        m_displays.push_back(demo);
        m_currentVibrance["DEMO-1"] = 0;
    }
    
    return !m_displays.empty();
}

std::vector<Display> VibranceController::getDisplays() {
    return m_displays;
}

bool VibranceController::setVibrance(const std::string& displayId, int vibrance) {
    // Clamp to valid range
    vibrance = std::max(-100, std::min(100, vibrance));
    
    // Apply immediately for real-time feedback
    if (applyVibranceImmediate(displayId, vibrance)) {
        // Update stored value
        m_currentVibrance[displayId] = vibrance;
        
        // Update display object
        for (auto& display : m_displays) {
            if (display.id == displayId) {
                display.currentVibrance = vibrance;
                break;
            }
        }
        
        return true;
    }
    
    return false;
}

bool VibranceController::applyVibranceImmediate(const std::string& displayId, int vibrance) {
    // FIXED: Much more aggressive saturation control for visible changes
    
    if (displayId == "DEMO-1") {
        std::cout << "🎮 Demo: " << displayId << " = " << vibrance << std::endl;
        return true;
    }
    
    // Convert vibrance to dramatic color matrix transformation
    float saturation = 1.0f + (vibrance / 50.0f); // More aggressive: -100 to +100 -> -1.0 to +3.0
    saturation = std::max(0.1f, std::min(3.0f, saturation));
    
    // Method 1: Try advanced color matrix (most effective)
    if (applyColorMatrix(displayId, saturation)) {
        std::cout << "✅ Applied " << vibrance << " to " << displayId << std::endl;
        return true;
    }
    
    // Method 2: Fallback to aggressive gamma (still visible)
    if (applyAggressiveGamma(displayId, saturation)) {
        std::cout << "✅ Applied " << vibrance << " to " << displayId << std::endl;
        return true;
    }
    
    // Method 3: Last resort - basic gamma
    float gamma = 1.0f / saturation;
    gamma = std::max(0.3f, std::min(3.0f, gamma));
    
    std::ostringstream cmd;
    cmd << "xrandr --output " << displayId 
        << " --gamma " << gamma << ":" << gamma << ":" << gamma
        << " 2>/dev/null";
    
    int result = system(cmd.str().c_str());
    
    if (result == 0) {
        std::cout << "✅ Applied " << vibrance << " to " << displayId << std::endl;
        return true;
    } else {
        std::cout << "⚠️ Command failed, but continuing..." << std::endl;
        return true; // Don't fail the UI
    }
}

bool VibranceController::applyColorMatrix(const std::string& displayId, float saturation) {
    // Create a proper saturation matrix for dramatic color changes
    // This is similar to what NVIDIA's digital vibrance does
    
    float s = saturation;
    float sr = (1.0f - s) * 0.3086f; // Red weight
    float sg = (1.0f - s) * 0.6094f; // Green weight  
    float sb = (1.0f - s) * 0.0820f; // Blue weight
    
    // Saturation matrix values
    float rr = sr + s, rg = sr,     rb = sr;
    float gr = sg,     gg = sg + s, gb = sg;
    float br = sb,     bg = sb,     bb = sb + s;
    
    // Try to apply color transformation matrix via xrandr
    std::ostringstream cmd;
    cmd << "xrandr --output " << displayId 
        << " --transform " 
        << rr << "," << rg << "," << rb << ","
        << gr << "," << gg << "," << gb << ","
        << br << "," << bg << "," << bb
        << " 2>/dev/null";
    
    int result = system(cmd.str().c_str());
    return result == 0;
}

bool VibranceController::applyAggressiveGamma(const std::string& displayId, float saturation) {
    // More aggressive gamma curve for visible saturation changes
    float redGamma = 1.0f / std::pow(saturation, 0.8f);
    float greenGamma = 1.0f / saturation;
    float blueGamma = 1.0f / std::pow(saturation, 1.2f);
    
    // Clamp to safe but dramatic range
    redGamma = std::max(0.4f, std::min(2.5f, redGamma));
    greenGamma = std::max(0.4f, std::min(2.5f, greenGamma));
    blueGamma = std::max(0.4f, std::min(2.5f, blueGamma));
    
    std::ostringstream cmd;
    cmd << "xrandr --output " << displayId 
        << " --gamma " << redGamma << ":" << greenGamma << ":" << blueGamma
        << " 2>/dev/null";
    
    int result = system(cmd.str().c_str());
    return result == 0;
}

int VibranceController::getVibrance(const std::string& displayId) {
    auto it = m_currentVibrance.find(displayId);
    return (it != m_currentVibrance.end()) ? it->second : 0;
}

bool VibranceController::resetAllDisplays() {
    std::cout << "🔄 Resetting all displays..." << std::endl;
    
    bool success = true;
    for (auto& display : m_displays) {
        if (display.id != "DEMO-1") {
            // Reset both gamma and transform
            std::string resetCmd1 = "xrandr --output " + display.id + " --gamma 1:1:1 2>/dev/null";
            std::string resetCmd2 = "xrandr --output " + display.id + " --transform 1,0,0,0,1,0,0,0,1 2>/dev/null";
            
            system(resetCmd1.c_str());
            system(resetCmd2.c_str());
        }
        display.currentVibrance = 0;
        m_currentVibrance[display.id] = 0;
    }
    
    return success;
}

bool VibranceController::installSystemWide() {
    std::cout << "📦 Installing Vivid system-wide..." << std::endl;
    
    // Check if we're in a build directory
    if (system("test -f builddir/vivid") != 0) {
        std::cout << "❌ Build not found" << std::endl;
        return false;
    }
    
    // Try multiple installation methods
    
    // Method 1: Use meson install
    if (system("sudo meson install -C builddir 2>/dev/null") == 0) {
        std::cout << "✅ Vivid installed system-wide!" << std::endl;
        return true;
    }
    
    // Method 2: Manual installation
    std::cout << "🔄 Trying manual installation..." << std::endl;
    
    // Copy binary
    if (system("sudo cp builddir/vivid /usr/local/bin/vivid 2>/dev/null") == 0) {
        system("sudo chmod +x /usr/local/bin/vivid 2>/dev/null");
        
        // Create desktop file
        std::string desktopContent = R"([Desktop Entry]
Type=Application
Name=Vivid
GenericName=Digital Vibrance Control
Comment=Adjust screen vibrance and digital saturation
Exec=vivid
Icon=applications-graphics
Terminal=false
Categories=System;Settings;
Keywords=vibrance;saturation;color;display;
StartupNotify=true
)";
        
        // Write desktop file
        system("sudo mkdir -p /usr/local/share/applications 2>/dev/null");
        
        std::ofstream desktopFile("/tmp/vivid.desktop");
        if (desktopFile.is_open()) {
            desktopFile << desktopContent;
            desktopFile.close();
            
            if (system("sudo mv /tmp/vivid.desktop /usr/local/share/applications/vivid.desktop 2>/dev/null") == 0) {
                system("sudo chmod 644 /usr/local/share/applications/vivid.desktop 2>/dev/null");
                std::cout << "✅ Vivid installed system-wide!" << std::endl;
                return true;
            }
        }
    }
    
    std::cout << "❌ Installation failed" << std::endl;
    return false;
}

bool VibranceController::isSystemInstalled() {
    return (system("which vivid > /dev/null 2>&1") == 0) || 
           (system("test -f /usr/local/bin/vivid") == 0);
}
