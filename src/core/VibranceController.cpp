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
    resetAllDisplays();
}

bool VibranceController::initialize() {
    if (!detectDisplays()) {
        return false;
    }
    
    m_initialized = true;
    return true;
}

bool VibranceController::detectDisplays() {
    m_displays.clear();
    
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
        }
    }
    pclose(pipe);
    
    if (m_displays.empty()) {
        Display demo;
        demo.id = "eDP-1";
        demo.name = "Built-in Display";
        demo.currentVibrance = 0;
        m_displays.push_back(demo);
        m_currentVibrance["eDP-1"] = 0;
    }
    
    return !m_displays.empty();
}

std::vector<Display> VibranceController::getDisplays() {
    return m_displays;
}

bool VibranceController::setVibrance(const std::string& displayId, int vibrance) {
    vibrance = std::max(-100, std::min(100, vibrance));
    
    if (applyVibranceImmediate(displayId, vibrance)) {
        m_currentVibrance[displayId] = vibrance;
        
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
    // Method 1: Try xgamma (most effective for saturation)
    if (applyXGamma(displayId, vibrance)) {
        return true;
    }
    
    // Method 2: Try redshift approach
    if (applyRedshift(vibrance)) {
        return true;
    }
    
    // Method 3: Try xcalib
    if (applyXCalib(displayId, vibrance)) {
        return true;
    }
    
    // Method 4: Fallback to xrandr
    return applyXRandr(displayId, vibrance);
}

bool VibranceController::applyXGamma(const std::string& displayId, int vibrance) {
    // xgamma is more effective for color changes
    float factor = 1.0f + (vibrance / 100.0f);
    factor = std::max(0.1f, std::min(3.0f, factor));
    
    // Create different gamma values for RGB to simulate saturation
    float red = std::pow(factor, 0.8f);
    float green = factor;
    float blue = std::pow(factor, 1.2f);
    
    red = std::max(0.1f, std::min(3.0f, red));
    green = std::max(0.1f, std::min(3.0f, green));
    blue = std::max(0.1f, std::min(3.0f, blue));
    
    std::ostringstream cmd;
    cmd << "DISPLAY=:0 xgamma -rgamma " << red << " -ggamma " << green << " -bgamma " << blue << " 2>/dev/null";
    
    return system(cmd.str().c_str()) == 0;
}

bool VibranceController::applyRedshift(int vibrance) {
    // Use redshift for color temperature adjustment
    if (system("which redshift > /dev/null 2>&1") != 0) {
        return false;
    }
    
    // Kill existing redshift
    system("pkill redshift 2>/dev/null");
    
    if (vibrance == 0) {
        system("redshift -x 2>/dev/null");
        return true;
    }
    
    // Convert vibrance to color temperature and brightness
    int temp = 6500 + (vibrance * 15); // 5000-8000K range
    float brightness = 1.0f + (vibrance / 200.0f); // 0.5-1.5 range
    
    temp = std::max(3000, std::min(10000, temp));
    brightness = std::max(0.3f, std::min(2.0f, brightness));
    
    std::ostringstream cmd;
    cmd << "redshift -O " << temp << " -b " << brightness << " 2>/dev/null &";
    
    return system(cmd.str().c_str()) == 0;
}

bool VibranceController::applyXCalib(const std::string& displayId, int vibrance) {
    if (system("which xcalib > /dev/null 2>&1") != 0) {
        return false;
    }
    
    if (vibrance == 0) {
        std::string cmd = "xcalib -clear 2>/dev/null";
        return system(cmd.c_str()) == 0;
    }
    
    // Create a temporary ICC profile for saturation
    std::string tempProfile = "/tmp/vivid_profile.icc";
    
    // Generate basic saturation adjustment
    float sat = 1.0f + (vibrance / 100.0f);
    sat = std::max(0.1f, std::min(2.0f, sat));
    
    std::ostringstream cmd;
    cmd << "xcalib -alter -gamma " << sat << " 2>/dev/null";
    
    return system(cmd.str().c_str()) == 0;
}

bool VibranceController::applyXRandr(const std::string& displayId, int vibrance) {
    float factor = 1.0f + (vibrance / 100.0f);
    factor = std::max(0.3f, std::min(2.0f, factor));
    
    float gamma = 1.0f / factor;
    gamma = std::max(0.5f, std::min(3.0f, gamma));
    
    std::ostringstream cmd;
    cmd << "xrandr --output " << displayId << " --gamma " << gamma << ":" << gamma << ":" << gamma << " 2>/dev/null";
    
    return system(cmd.str().c_str()) == 0;
}

int VibranceController::getVibrance(const std::string& displayId) {
    auto it = m_currentVibrance.find(displayId);
    return (it != m_currentVibrance.end()) ? it->second : 0;
}

bool VibranceController::resetAllDisplays() {
    bool success = true;
    
    // Reset xgamma
    system("DISPLAY=:0 xgamma -gamma 1.0 2>/dev/null");
    
    // Reset redshift
    system("pkill redshift 2>/dev/null");
    system("redshift -x 2>/dev/null");
    
    // Reset xcalib
    system("xcalib -clear 2>/dev/null");
    
    // Reset xrandr
    for (auto& display : m_displays) {
        std::string cmd = "xrandr --output " + display.id + " --gamma 1:1:1 2>/dev/null";
        system(cmd.c_str());
        
        display.currentVibrance = 0;
        m_currentVibrance[display.id] = 0;
    }
    
    return success;
}

bool VibranceController::installSystemWide() {
    if (system("test -f builddir/vivid") != 0) {
        return false;
    }
    
    // Try pkexec for GUI password prompt
    if (system("pkexec cp builddir/vivid /usr/local/bin/vivid 2>/dev/null") == 0) {
        system("pkexec chmod +x /usr/local/bin/vivid 2>/dev/null");
        return true;
    }
    
    // Fallback to sudo
    if (system("sudo cp builddir/vivid /usr/local/bin/vivid 2>/dev/null") == 0) {
        system("sudo chmod +x /usr/local/bin/vivid 2>/dev/null");
        return true;
    }
    
    return false;
}

bool VibranceController::isSystemInstalled() {
    return system("which vivid > /dev/null 2>&1") == 0;
}
