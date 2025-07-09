#include "VibranceController.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <chrono>
#include <algorithm>

VibranceController::VibranceController() {
    initialize();
}

VibranceController::~VibranceController() {
    // Reset all displays to original values on exit
    for (const auto& display : m_displays) {
        resetDisplay(display.id);
    }
}

bool VibranceController::initialize() {
    std::cout << "🎮 Initializing Vivid Vibrance Controller..." << std::endl;
    
    if (!detectDisplays()) {
        std::cerr << "❌ Failed to detect displays" << std::endl;
        return false;
    }
    
    // Store original vibrance values
    for (const auto& display : m_displays) {
        m_baseVibrance[display.id] = 0; // Assume 0 as baseline
    }
    
    loadSettings();
    m_initialized = true;
    
    std::cout << "✅ Vivid initialized successfully!" << std::endl;
    std::cout << "   Found " << m_displays.size() << " display(s)" << std::endl;
    
    return true;
}

bool VibranceController::detectDisplays() {
    m_displays.clear();
    
    // Use xrandr to detect connected displays
    FILE* pipe = popen("xrandr --listmonitors 2>/dev/null", "r");
    if (!pipe) {
        std::cerr << "❌ Failed to run xrandr" << std::endl;
        return false;
    }
    
    char buffer[256];
    bool foundDisplays = false;
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        
        // Skip header line
        if (line.find("Monitors:") != std::string::npos) continue;
        
        // Parse monitor line: " 0: +*DVI-D-0 1920/510x1080/287+0+0  DVI-D-0"
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;
        
        // Extract display name (last part)
        std::istringstream iss(line);
        std::string token;
        std::string displayId;
        
        while (iss >> token) {
            if (token.find("DVI") != std::string::npos || 
                token.find("HDMI") != std::string::npos || 
                token.find("DP") != std::string::npos ||
                token.find("eDP") != std::string::npos ||
                token.find("VGA") != std::string::npos) {
                displayId = token;
            }
        }
        
        if (!displayId.empty()) {
            Display display;
            display.id = displayId;
            display.name = displayId;
            display.currentVibrance = 0;
            display.connected = true;
            
            m_displays.push_back(display);
            foundDisplays = true;
            
            std::cout << "   📺 Found display: " << displayId << std::endl;
        }
    }
    
    pclose(pipe);
    
    // Fallback for testing
    if (!foundDisplays) {
        std::cout << "⚠️  No displays detected, adding demo displays" << std::endl;
        
        Display demo1;
        demo1.id = "DVI-D-0";
        demo1.name = "DVI-D-0";
        demo1.currentVibrance = 0;
        m_displays.push_back(demo1);
        
        Display demo2;
        demo2.id = "HDMI-0";
        demo2.name = "HDMI-0";
        demo2.currentVibrance = 0;
        m_displays.push_back(demo2);
        
        foundDisplays = true;
    }
    
    return foundDisplays;
}

std::vector<Display> VibranceController::getDisplays() {
    return m_displays;
}

bool VibranceController::setVibrance(const std::string& displayId, int vibrance) {
    // Clamp vibrance to valid range
    vibrance = std::max(-100, std::min(100, vibrance));
    
    std::cout << "🎨 Setting " << displayId << " vibrance to " << vibrance << std::endl;
    
    bool success = false;
    
    // Try AMD-specific method first
    if (applyVibranceAMD(displayId, vibrance)) {
        success = true;
        std::cout << "   ✅ Applied via AMD method" << std::endl;
    }
    // Fallback to xrandr gamma adjustment
    else if (applyVibranceXrandr(displayId, vibrance)) {
        success = true;
        std::cout << "   ✅ Applied via xrandr gamma" << std::endl;
    }
    else {
        std::cout << "   ❌ Failed to apply vibrance" << std::endl;
    }
    
    if (success) {
        // Update stored value
        for (auto& display : m_displays) {
            if (display.id == displayId) {
                display.currentVibrance = vibrance;
                break;
            }
        }
        
        // Save settings immediately for persistence
        saveSettings();
    }
    
    return success;
}

bool VibranceController::applyVibranceAMD(const std::string& displayId, int vibrance) {
    // Check if AMD GPU is present
    std::ifstream vendorFile("/sys/class/drm/card0/device/vendor");
    if (vendorFile.is_open()) {
        std::string vendor;
        std::getline(vendorFile, vendor);
        if (vendor == "0x1002") { // AMD vendor ID
            // Try to use AMD-specific vibrance control
            // This would require more complex implementation with DRM/KMS
            std::cout << "   🔍 AMD GPU detected, trying advanced method..." << std::endl;
            
            // For now, fall back to xrandr
            return false;
        }
    }
    return false;
}

bool VibranceController::applyVibranceXrandr(const std::string& displayId, int vibrance) {
    // Convert vibrance (-100 to +100) to gamma values
    // Vibrance affects color saturation, we'll use gamma as approximation
    
    float vibranceFactor = 1.0f + (vibrance / 100.0f * 0.5f); // Scale to 0.5-1.5 range
    vibranceFactor = std::max(0.5f, std::min(1.5f, vibranceFactor));
    
    // Apply different gamma values to R, G, B for vibrance effect
    float redGamma = vibranceFactor;
    float greenGamma = 1.0f;
    float blueGamma = vibranceFactor;
    
    // Build xrandr command
    std::ostringstream cmd;
    cmd << "xrandr --output " << displayId 
        << " --gamma " << redGamma << ":" << greenGamma << ":" << blueGamma
        << " 2>/dev/null";
    
    std::string command = cmd.str();
    std::cout << "   🔧 Running: " << command << std::endl;
    
    int result = system(command.c_str());
    return result == 0;
}

int VibranceController::getVibrance(const std::string& displayId) {
    for (const auto& display : m_displays) {
        if (display.id == displayId) {
            return display.currentVibrance;
        }
    }
    return 0;
}

bool VibranceController::resetDisplay(const std::string& displayId) {
    std::cout << "🔄 Resetting " << displayId << " to normal" << std::endl;
    
    // Reset gamma to 1:1:1
    std::string command = "xrandr --output " + displayId + " --gamma 1:1:1 2>/dev/null";
    int result = system(command.c_str());
    
    if (result == 0) {
        // Update stored value
        for (auto& display : m_displays) {
            if (display.id == displayId) {
                display.currentVibrance = 0;
                break;
            }
        }
        saveSettings();
        return true;
    }
    
    return false;
}

void VibranceController::saveProfile(const ProgramProfile& profile) {
    // Remove existing profile with same name
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&](const ProgramProfile& p) { return p.name == profile.name; });
    
    if (it != m_profiles.end()) {
        *it = profile;
    } else {
        m_profiles.push_back(profile);
    }
    
    saveSettings();
    std::cout << "💾 Saved profile: " << profile.name << std::endl;
}

void VibranceController::removeProfile(const std::string& name) {
    auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
                          [&](const ProgramProfile& p) { return p.name == name; });
    
    if (it != m_profiles.end()) {
        m_profiles.erase(it);
        saveSettings();
        std::cout << "🗑️ Removed profile: " << name << std::endl;
    }
}

std::vector<ProgramProfile> VibranceController::getProfiles() {
    return m_profiles;
}

void VibranceController::setFocusMode(bool enabled) {
    if (enabled && !m_focusMode) {
        startMonitoring();
    } else if (!enabled && m_focusMode) {
        stopMonitoring();
    }
    
    m_focusMode = enabled;
    saveSettings();
    
    std::cout << "🎯 Focus mode: " << (enabled ? "enabled" : "disabled") << std::endl;
}

void VibranceController::startMonitoring() {
    std::thread([this]() {
        while (m_focusMode) {
            checkActiveProgram();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
}

void VibranceController::stopMonitoring() {
    // Reset all displays when stopping focus mode
    for (const auto& display : m_displays) {
        resetDisplay(display.id);
    }
}

void VibranceController::checkActiveProgram() {
    std::string activeWindow = getActiveWindow();
    
    for (const auto& profile : m_profiles) {
        if (!profile.enabled) continue;
        
        bool matches = false;
        
        if (profile.pathMatching) {
            matches = activeWindow.find(profile.path) != std::string::npos;
        } else {
            matches = activeWindow.find(profile.windowTitle) != std::string::npos;
        }
        
        if (matches) {
            // Apply profile vibrance settings
            for (const auto& setting : profile.displayVibrance) {
                setVibrance(setting.first, setting.second);
            }
            return;
        }
    }
    
    // No matching profile, reset to base vibrance
    for (const auto& display : m_displays) {
        if (display.currentVibrance != 0) {
            resetDisplay(display.id);
        }
    }
}

std::string VibranceController::getActiveWindow() {
    FILE* pipe = popen("xdotool getactivewindow getwindowname 2>/dev/null", "r");
    if (!pipe) return "";
    
    char buffer[256];
    std::string result;
    if (fgets(buffer, sizeof(buffer), pipe)) {
        result = buffer;
        // Remove newline
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
    }
    pclose(pipe);
    
    return result;
}

std::string VibranceController::getConfigPath() {
    std::string home = std::getenv("HOME");
    return home + "/.config/vivid/settings.conf";
}

void VibranceController::saveSettings() {
    std::string configPath = getConfigPath();
    std::filesystem::create_directories(std::filesystem::path(configPath).parent_path());
    
    std::ofstream file(configPath);
    if (!file.is_open()) return;
    
    // Save focus mode
    file << "focus_mode=" << (m_focusMode ? "1" : "0") << std::endl;
    
    // Save display vibrance
    for (const auto& display : m_displays) {
        file << "display_" << display.id << "=" << display.currentVibrance << std::endl;
    }
    
    // Save profiles
    for (const auto& profile : m_profiles) {
        file << "profile_start=" << profile.name << std::endl;
        file << "profile_path=" << profile.path << std::endl;
        file << "profile_title=" << profile.windowTitle << std::endl;
        file << "profile_pathmatching=" << (profile.pathMatching ? "1" : "0") << std::endl;
        file << "profile_enabled=" << (profile.enabled ? "1" : "0") << std::endl;
        
        for (const auto& setting : profile.displayVibrance) {
            file << "profile_vibrance_" << setting.first << "=" << setting.second << std::endl;
        }
        
        file << "profile_end" << std::endl;
    }
    
    file.close();
}

void VibranceController::loadSettings() {
    std::string configPath = getConfigPath();
    std::ifstream file(configPath);
    if (!file.is_open()) return;
    
    std::string line;
    ProgramProfile currentProfile;
    bool inProfile = false;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        if (key == "focus_mode") {
            m_focusMode = (value == "1");
        }
        else if (key.substr(0, 8) == "display_") {
            std::string displayId = key.substr(8);
            int vibrance = std::stoi(value);
            
            // Apply saved vibrance
            setVibrance(displayId, vibrance);
        }
        else if (key == "profile_start") {
            currentProfile = ProgramProfile();
            currentProfile.name = value;
            inProfile = true;
        }
        else if (key == "profile_end" && inProfile) {
            m_profiles.push_back(currentProfile);
            inProfile = false;
        }
        else if (inProfile) {
            if (key == "profile_path") currentProfile.path = value;
            else if (key == "profile_title") currentProfile.windowTitle = value;
            else if (key == "profile_pathmatching") currentProfile.pathMatching = (value == "1");
            else if (key == "profile_enabled") currentProfile.enabled = (value == "1");
            else if (key.substr(0, 17) == "profile_vibrance_") {
                std::string displayId = key.substr(17);
                int vibrance = std::stoi(value);
                currentProfile.displayVibrance[displayId] = vibrance;
            }
        }
    }
    
    file.close();
    
    if (m_focusMode) {
        startMonitoring();
    }
}
