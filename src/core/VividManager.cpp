#include "VividManager.h"
#include "AutostartManager.h"
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
using X11Display = Display;
#endif

VividManager::VividManager() 
    : m_currentMethod(VibranceMethod::DEMO_MODE)
    , m_initialized(false)
    , m_monitoringEnabled(false) {
    m_autostartManager = std::make_unique<AutostartManager>();
}

VividManager::~VividManager() {
    stopApplicationMonitoring();
    
    // Safety: Reset all displays to original values on exit
    std::cout << "🛡️ Safety shutdown: Resetting all displays..." << std::endl;
    for (const auto& display : m_displays) {
        resetVibrance(display.id);
    }
}

bool VividManager::initialize() {
    std::cout << "Initializing Vivid Manager..." << std::endl;
    
    // Store original vibrance values for safety
    detectDisplays();
    for (const auto& display : m_displays) {
        m_originalVibrance[display.id] = 0.0f; // Assume normal as original
    }
    
    // Detect session type
    std::string sessionType = "unknown";
    if (std::getenv("WAYLAND_DISPLAY")) {
        sessionType = "wayland";
        std::cout << "  Detected Wayland session" << std::endl;
    } else if (std::getenv("DISPLAY")) {
        sessionType = "x11";
        std::cout << "  Detected X11 session" << std::endl;
    }
    
    // Try methods with safety priority
    if (tryAMDColorProperties()) {
        m_currentMethod = VibranceMethod::AMD_COLOR_PROPERTIES;
        std::cout << "✓ Using AMD Color Properties method (Safe)" << std::endl;
    } else if (sessionType == "wayland" && tryWaylandColorMgmt()) {
        m_currentMethod = VibranceMethod::WAYLAND_COLOR_MGMT;
        std::cout << "✓ Using Wayland Color Management method" << std::endl;
    } else if (sessionType == "x11" && tryXRandrCTM()) {
        m_currentMethod = VibranceMethod::XRANDR_CTM;
        std::cout << "✓ Using XRandR method (Limited)" << std::endl;
    } else {
        m_currentMethod = VibranceMethod::DEMO_MODE;
        std::cout << "⚠ Using demo mode - Interface testing only" << std::endl;
        std::cout << "  All controls work, but no actual display changes" << std::endl;
    }
    
    loadProfiles();
    m_initialized = true;
    
    return true;
}

bool VividManager::setVibranceSafe(const std::string& displayId, float vibrance) {
    // Safety limits - more conservative than the full range
    const float SAFE_MIN = -75.0f;  // Don't go too gray
    const float SAFE_MAX = 75.0f;   // Don't go too saturated
    
    // Clamp to safe range
    vibrance = std::max(SAFE_MIN, std::min(SAFE_MAX, vibrance));
    
    std::cout << "🛡️ Safe vibrance change: " << displayId << " -> " << vibrance << std::endl;
    
    // Call the regular setVibrance with safety-clamped values
    return setVibrance(displayId, vibrance);
}

bool VividManager::setVibrance(const std::string& displayId, float vibrance) {
    // Additional safety check
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
            success = true;
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
    // Convert vibrance to safe gamma adjustment
    float saturation = 1.0f + (vibrance / 100.0f);
    saturation = std::max(0.3f, std::min(1.7f, saturation)); // Conservative limits
    
    std::cout << "  Trying safe AMD vibrance control..." << std::endl;
    
    // Use gamma adjustment as it's safer than direct color properties
    float gamma = 1.0f / saturation;
    gamma = std::max(0.6f, std::min(1.6f, gamma)); // Very conservative gamma limits
    
    std::string command = "xrandr --output " + displayId + " --gamma " + 
                         std::to_string(gamma) + ":" + std::to_string(gamma) + ":" + std::to_string(gamma) + " 2>/dev/null";
    
    std::cout << "    Safe command: " << command << std::endl;
    int result = system(command.c_str());
    
    if (result == 0) {
        std::cout << "    ✅ Safe gamma adjustment successful" << std::endl;
        return true;
    } else {
        std::cout << "    ⚠ Gamma adjustment failed (this is normal in demo mode)" << std::endl;
        return false;
    }
}

bool VividManager::tryAMDColorProperties() {
    std::cout << "  Checking for AMD GPU..." << std::endl;
    
    std::ifstream drmFile("/sys/class/drm/card0/device/vendor");
    if (drmFile.is_open()) {
        std::string vendor;
        std::getline(drmFile, vendor);
        if (vendor == "0x1002") {
            std::cout << "    ✓ AMD GPU detected" << std::endl;
            if (std::filesystem::exists("/sys/module/amdgpu")) {
                std::cout << "    ✓ AMDGPU driver loaded" << std::endl;
                return tryAMDXrandrFallback();
            }
        }
    }
    return false;
}

bool VividManager::tryAMDXrandrFallback() {
    std::cout << "    Checking safe xrandr availability..." << std::endl;
    int result = system("which xrandr > /dev/null 2>&1");
    if (result == 0) {
        result = system("xrandr --listmonitors > /dev/null 2>&1");
        if (result == 0) {
            std::cout << "    ✅ Safe xrandr method available" << std::endl;
            return true;
        }
    }
    return false;
}

bool VividManager::tryXRandrCTM() {
    return false; // Disabled for safety
}

bool VividManager::tryWaylandColorMgmt() {
    if (!std::getenv("WAYLAND_DISPLAY")) return false;
    std::cout << "    ⚠ Wayland color management not yet implemented" << std::endl;
    return false;
}

void VividManager::detectDisplays() {
    m_displays.clear();
    std::cout << "  Detecting displays safely..." << std::endl;
    
    bool foundRealDisplays = false;
    
    // Try xrandr first
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
    
    // Fallback to demo displays
    if (!foundRealDisplays) {
        std::cout << "    Using demo displays for interface testing" << std::endl;
        
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
    
    std::cout << "    Total displays: " << m_displays.size() << std::endl;
}

std::vector<VividDisplay> VividManager::getDisplays() {
    return m_displays;
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
    std::cout << "🔄 Resetting " << displayId << " to normal vibrance" << std::endl;
    return setVibrance(displayId, 0.0f);
}

std::string VividManager::getMethodName() const {
    switch (m_currentMethod) {
        case VibranceMethod::AMD_COLOR_PROPERTIES:
            return "AMD Safe Mode";
        case VibranceMethod::XRANDR_CTM:
            return "XRandR Limited";
        case VibranceMethod::WAYLAND_COLOR_MGMT:
            return "Wayland Color Management";
        case VibranceMethod::DEMO_MODE:
            return "Demo Mode (Interface Testing)";
    }
    return "Unknown";
}

bool VividManager::setXRandrVibrance(const std::string& displayId __attribute__((unused)), float vibrance __attribute__((unused))) {
    return false;
}

bool VividManager::setWaylandVibrance(const std::string& displayId __attribute__((unused)), float vibrance __attribute__((unused))) {
    return false;
}

// Profile management
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
    m_monitoringEnabled = enabled;
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

// Autostart functionality
bool VividManager::isAutostartEnabled() {
    return m_autostartManager->isEnabled();
}

bool VividManager::enableAutostart() {
    return m_autostartManager->enable();
}

bool VividManager::disableAutostart() {
    return m_autostartManager->disable();
}

std::string VividManager::getAutostartStatus() {
    return m_autostartManager->getStatus();
}
