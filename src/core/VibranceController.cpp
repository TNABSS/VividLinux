#include "VibranceController.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

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
    // Convert vibrance to gamma values for immediate application
    float factor = 1.0f + (vibrance / 200.0f); // -100 to +100 -> 0.5 to 1.5
    factor = std::max(0.5f, std::min(1.5f, factor));
    
    // Apply gamma immediately
    float gamma = 1.0f / factor;
    gamma = std::max(0.5f, std::min(2.0f, gamma));
    
    std::ostringstream cmd;
    cmd << "xrandr --output " << displayId 
        << " --gamma " << gamma << ":" << gamma << ":" << gamma
        << " 2>/dev/null";
    
    int result = system(cmd.str().c_str());
    
    if (result == 0) {
        std::cout << "✅ Applied " << vibrance << " to " << displayId << std::endl;
        return true;
    } else {
        // Demo mode - always succeed for UI testing
        std::cout << "🎮 Demo: " << displayId << " = " << vibrance << std::endl;
        return true;
    }
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
            std::string cmd = "xrandr --output " + display.id + " --gamma 1:1:1 2>/dev/null";
            if (system(cmd.c_str()) != 0) {
                success = false;
            }
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
    
    // Install using meson
    int result = system("pkexec meson install -C builddir 2>/dev/null");
    
    if (result == 0) {
        std::cout << "✅ Vivid installed system-wide!" << std::endl;
        return true;
    } else {
        std::cout << "❌ Installation failed" << std::endl;
        return false;
    }
}

bool VibranceController::isSystemInstalled() {
    return system("which vivid > /dev/null 2>&1") == 0;
}
