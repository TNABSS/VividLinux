#include "VividManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <chrono>
#include <regex>

#ifdef HAVE_X11
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xatom.h>
// Use X11::Display to avoid conflict
using X11Display = Display;
#endif

VividManager::VividManager() 
    : m_currentMethod(VibranceMethod::DEMO_MODE)
    , m_initialized(false)
    , m_monitoringEnabled(false) {
}

VividManager::~VividManager() {
    stopApplicationMonitoring();
}

bool VividManager::initialize() {
    std::cout << "Initializing Vivid Manager..." << std::endl;
    
    // Detect session type first
    std::string sessionType = "unknown";
    if (std::getenv("WAYLAND_DISPLAY")) {
        sessionType = "wayland";
        std::cout << "  Detected Wayland session" << std::endl;
    } else if (std::getenv("DISPLAY")) {
        sessionType = "x11";
        std::cout << "  Detected X11 session" << std::endl;
    }
    
    // Try different methods in order of preference for your setup
    if (tryAMDColorProperties()) {
        m_currentMethod = VibranceMethod::AMD_COLOR_PROPERTIES;
        std::cout << "✓ Using AMD Color Properties method" << std::endl;
    } else if (sessionType == "wayland" && tryWaylandColorMgmt()) {
        m_currentMethod = VibranceMethod::WAYLAND_COLOR_MGMT;
        std::cout << "✓ Using Wayland Color Management method" << std::endl;
    } else if (sessionType == "x11" && tryXRandrCTM()) {
        m_currentMethod = VibranceMethod::XRANDR_CTM;
        std::cout << "✓ Using XRandR Color Transformation Matrix method" << std::endl;
    } else {
        m_currentMethod = VibranceMethod::DEMO_MODE;
        std::cout << "⚠ Using demo mode - GUI will work but vibrance changes are simulated" << std::endl;
        std::cout << "  This is normal for testing the interface!" << std::endl;
    }
    
    detectDisplays();
    loadProfiles();
    m_initialized = true;
    
    return true;
}

bool VividManager::tryAMDColorProperties() {
    std::cout << "  Checking for AMD GPU..." << std::endl;
    
    // Check if we're using AMD drivers
    std::ifstream drmFile("/sys/class/drm/card0/device/vendor");
    if (drmFile.is_open()) {
        std::string vendor;
        std::getline(drmFile, vendor);
        if (vendor == "0x1002") { // AMD vendor ID
            std::cout << "    ✓ AMD GPU detected (vendor: " << vendor << ")" << std::endl;
            
            // Check for AMDGPU driver
            if (std::filesystem::exists("/sys/module/amdgpu")) {
                std::cout << "    ✓ AMDGPU driver loaded" << std::endl;
                
                // For now, we'll implement basic AMD support via xrandr fallback
                // Real AMD color properties would need DRM/KMS integration
                return tryAMDXrandrFallback();
            } else {
                std::cout << "    ⚠ AMDGPU driver not loaded" << std::endl;
            }
        } else {
            std::cout << "    ⚠ Non-AMD GPU detected (vendor: " << vendor << ")" << std::endl;
        }
    } else {
        std::cout << "    ⚠ Cannot detect GPU vendor" << std::endl;
    }
    return false;
}

bool VividManager::tryAMDXrandrFallback() {
    // Even on Wayland, some compositors support xrandr for compatibility
    std::cout << "    Checking xrandr availability..." << std::endl;
    
    int result = system("which xrandr > /dev/null 2>&1");
    if (result == 0) {
        // Test if xrandr can list outputs
        result = system("xrandr --listmonitors > /dev/null 2>&1");
        if (result == 0) {
            std::cout << "    ✓ xrandr working (even on Wayland!)" << std::endl;
            return true;
        }
    }
    
    std::cout << "    ⚠ xrandr not available" << std::endl;
    return false;
}

bool VividManager::tryXRandrCTM() {
    // Skip X11-specific CTM for Wayland users
    return false;
}

bool VividManager::tryWaylandColorMgmt() {
    std::cout << "  Checking Wayland color management..." << std::endl;
    
    // Check if we're in a Wayland session
    if (!std::getenv("WAYLAND_DISPLAY")) {
        std::cout << "    ⚠ Not in Wayland session" << std::endl;
        return false;
    }
    
    // Check for common Wayland compositors that might support color management
    std::string compositor = "unknown";
    if (std::getenv("GNOME_DESKTOP_SESSION_ID")) {
        compositor = "gnome";
    } else if (std::getenv("KDE_SESSION_VERSION")) {
        compositor = "kde";
    } else if (std::getenv("SWAY_SOCK")) {
        compositor = "sway";
    }
    
    std::cout << "    Detected compositor: " << compositor << std::endl;
    
    // For now, Wayland color management is not fully implemented
    // This would require implementing the color-management-v1 protocol
    std::cout << "    ⚠ Wayland color management not yet implemented" << std::endl;
    std::cout << "    ⚠ Will use demo mode for interface testing" << std::endl;
    
    return false;
}

void VividManager::detectDisplays() {
    m_displays.clear();
    std::cout << "  Detecting displays..." << std::endl;
    
    // Try to get real display info first
    bool foundRealDisplays = false;
    
    // Method 1: Try xrandr (works on some Wayland compositors)
    if (system("which xrandr > /dev/null 2>&1") == 0) {
        FILE* pipe = popen("xrandr --listmonitors 2>/dev/null | grep -v '^Monitors:' | awk '{print $4}'", "r");
        if (pipe) {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                std::string displayName = buffer;
                displayName.erase(displayName.find_last_not_of(" \n\r\t") + 1);
                if (!displayName.empty()) {
                    VividDisplay display;
                    display.id = displayName;
                    display.name = displayName;
                    display.connector = displayName;
                    display.connected = true;
                    display.currentVibrance = 0.0f;
                    m_displays.push_back(display);
                    m_baseVibrance[display.id] = 0.0f;
                    foundRealDisplays = true;
                    std::cout << "    Found display: " << displayName << std::endl;
                }
            }
            pclose(pipe);
        }
    }
    
    // Method 2: Try DRM detection for AMD
    if (!foundRealDisplays) {
        for (int i = 0; i < 4; i++) {
            std::string cardPath = "/sys/class/drm/card" + std::to_string(i);
            if (std::filesystem::exists(cardPath)) {
                // Look for connected outputs
                for (const auto& entry : std::filesystem::directory_iterator(cardPath)) {
                    std::string name = entry.path().filename().string();
                    if (name.find("card" + std::to_string(i) + "-") == 0) {
                        std::string statusFile = entry.path().string() + "/status";
                        std::ifstream status(statusFile);
                        if (status.is_open()) {
                            std::string statusStr;
                            std::getline(status, statusStr);
                            if (statusStr == "connected") {
                                std::string displayName = name.substr(name.find('-') + 1);
                                VividDisplay display;
                                display.id = displayName;
                                display.name = displayName;
                                display.connector = displayName;
                                display.connected = true;
                                display.currentVibrance = 0.0f;
                                m_displays.push_back(display);
                                m_baseVibrance[display.id] = 0.0f;
                                foundRealDisplays = true;
                                std::cout << "    Found DRM display: " << displayName << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Fallback: Add demo displays for testing
    if (!foundRealDisplays) {
        std::cout << "    No real displays detected, using demo displays" << std::endl;
        
        VividDisplay display1;
        display1.id = "eDP-1";
        display1.name = "Built-in Display";
        display1.connector = "eDP";
        display1.connected = true;
        display1.currentVibrance = 0.0f;
        m_displays.push_back(display1);
        
        VividDisplay display2;
        display2.id = "HDMI-A-1";
        display2.name = "External Monitor";
        display2.connector = "HDMI-A";
        display2.connected = true;
        display2.currentVibrance = 0.0f;
        m_displays.push_back(display2);
        
        m_baseVibrance["eDP-1"] = 0.0f;
        m_baseVibrance["HDMI-A-1"] = 0.0f;
    }
    
    std::cout << "    Total displays found: " << m_displays.size() << std::endl;
}

std::vector<VividDisplay> VividManager::getDisplays() {
    return m_displays;
}

bool VividManager::setVibrance(const std::string& displayId, float vibrance) {
    // Clamp vibrance to -100 to +100 range
    vibrance = std::max(-100.0f, std::min(100.0f, vibrance));
    
    std::cout << "Setting " << displayId << " vibrance to " << vibrance << std::endl;
    
    bool success = false;
    switch (m_currentMethod) {
        case VibranceMethod::AMD_COLOR_PROPERTIES:
            success = setAMDVibrance(displayId, vibrance);
            break;
        case VibranceMethod::XRANDR_CTM:
            success = setXRandrVibrance(displayId, vibrance);
            break;
        case VibranceMethod::WAYLAND_COLOR_MGMT:
            success = setWaylandVibrance(displayId, vibrance);
            break;
        case VibranceMethod::DEMO_MODE:
            success = true; // Always succeed in demo mode
            std::cout << "  Demo mode: vibrance simulated at " << vibrance << std::endl;
            break;
    }
    
    if (success) {
        // Update display vibrance
        for (auto& display : m_displays) {
            if (display.id == displayId) {
                display.currentVibrance = vibrance;
                break;
            }
        }
    }
    
    return success;
}

bool VividManager::setAMDVibrance(const std::string& displayId, float vibrance) {
    // Convert vibrance (-100 to +100) to saturation multiplier (0.0 to 2.0)
    float saturation = 1.0f + (vibrance / 100.0f);
    saturation = std::max(0.0f, std::min(2.0f, saturation));
    
    std::cout << "  Trying AMD vibrance control..." << std::endl;
    
    // Method 1: Try xrandr gamma adjustment (works on many systems)
    float gamma = 1.0f / saturation;
    gamma = std::max(0.5f, std::min(2.0f, gamma));
    
    std::string command = "xrandr --output " + displayId + " --gamma " + 
                         std::to_string(gamma) + ":" + std::to_string(gamma) + ":" + std::to_string(gamma) + " 2>/dev/null";
    
    std::cout << "    Running: " << command << std::endl;
    int result = system(command.c_str());
    
    if (result == 0) {
        std::cout << "    ✓ xrandr gamma adjustment successful" << std::endl;
        return true;
    } else {
        std::cout << "    ⚠ xrandr gamma adjustment failed" << std::endl;
    }
    
    // Method 2: Try direct AMD sysfs control (future implementation)
    // This would involve writing to /sys/class/drm/card*/device/pp_od_clk_voltage
    
    return false;
}

bool VividManager::setXRandrVibrance(const std::string& displayId __attribute__((unused)), float vibrance __attribute__((unused))) {
    // X11-specific implementation - skip for Wayland
    return false;
}

bool VividManager::setWaylandVibrance(const std::string& displayId __attribute__((unused)), float vibrance __attribute__((unused))) {
    // Wayland color management implementation would go here
    // For now, return false as it's not implemented
    return false;
}

float VividManager::getVibrance(const std::string& displayId) {
    for (const auto& display : m_displays) {
        if (display.id == displayId) {
            return display.currentVibrance;
        }
    }
    return 0.0f;
}

bool VividManager::resetVibrance(const std::string& displayId) {
    return setVibrance(displayId, 0.0f);
}

std::string VividManager::getMethodName() const {
    switch (m_currentMethod) {
        case VibranceMethod::AMD_COLOR_PROPERTIES:
            return "AMD Color Properties";
        case VibranceMethod::XRANDR_CTM:
            return "XRandR Color Transformation";
        case VibranceMethod::WAYLAND_COLOR_MGMT:
            return "Wayland Color Management";
        case VibranceMethod::DEMO_MODE:
            return "Demo Mode (Interface Testing)";
    }
    return "Unknown";
}

// Profile management methods
bool VividManager::saveProfile(const AppProfile& profile) {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&](const AppProfile& p) { return p.name == profile.name; });
    
    if (it != m_profiles.end()) {
        *it = profile;
    } else {
        m_profiles.push_back(profile);
    }
    
    saveProfiles();
    return true;
}

bool VividManager::deleteProfile(const std::string& name) {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&](const AppProfile& p) { return p.name == name; });
    
    if (it != m_profiles.end()) {
        m_profiles.erase(it);
        saveProfiles();
        return true;
    }
    return false;
}

std::vector<AppProfile> VividManager::getProfiles() {
    return m_profiles;
}

AppProfile* VividManager::findProfile(const std::string& name) {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&](const AppProfile& p) { return p.name == name; });
    return (it != m_profiles.end()) ? &(*it) : nullptr;
}

void VividManager::loadProfiles() {
    // Load profiles from config file
    std::string configPath = getConfigPath();
    std::ifstream file(configPath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            // Simple profile parsing
        }
    }
}

void VividManager::saveProfiles() {
    std::string configPath = getConfigPath();
    std::filesystem::create_directories(std::filesystem::path(configPath).parent_path());
    
    std::ofstream file(configPath);
    if (file.is_open()) {
        for (const auto& profile : m_profiles) {
            file << "profile:" << profile.name << ":" << profile.executable << std::endl;
        }
    }
}

std::string VividManager::getConfigPath() {
    std::string home = std::getenv("HOME");
    return home + "/.config/vivid/profiles.conf";
}

void VividManager::setMonitoringEnabled(bool enabled) {
    if (enabled && !m_monitoringEnabled) {
        startApplicationMonitoring();
    } else if (!enabled && m_monitoringEnabled) {
        stopApplicationMonitoring();
    }
}

void VividManager::startApplicationMonitoring() {
    m_monitoringEnabled = true;
    std::thread([this]() {
        while (m_monitoringEnabled) {
            checkActiveApplication();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
}

void VividManager::stopApplicationMonitoring() {
    m_monitoringEnabled = false;
}

void VividManager::checkActiveApplication() {
    // Application monitoring implementation
}

std::string VividManager::getCurrentActiveWindow() {
    return "";
}
