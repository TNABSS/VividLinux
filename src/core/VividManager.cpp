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
    
    // Try different methods in order of preference
    if (tryAMDColorProperties()) {
        m_currentMethod = VibranceMethod::AMD_COLOR_PROPERTIES;
        std::cout << "✓ Using AMD Color Properties method" << std::endl;
    } else if (tryXRandrCTM()) {
        m_currentMethod = VibranceMethod::XRANDR_CTM;
        std::cout << "✓ Using XRandR Color Transformation Matrix method" << std::endl;
    } else if (tryWaylandColorMgmt()) {
        m_currentMethod = VibranceMethod::WAYLAND_COLOR_MGMT;
        std::cout << "✓ Using Wayland Color Management method" << std::endl;
    } else {
        m_currentMethod = VibranceMethod::DEMO_MODE;
        std::cout << "⚠ No hardware method available, using demo mode" << std::endl;
    }
    
    detectDisplays();
    loadProfiles();
    m_initialized = true;
    
    return true;
}

bool VividManager::tryAMDColorProperties() {
    // Check if we're using AMD drivers
    std::ifstream drmFile("/sys/class/drm/card0/device/vendor");
    if (drmFile.is_open()) {
        std::string vendor;
        std::getline(drmFile, vendor);
        if (vendor == "0x1002") { // AMD vendor ID
            std::cout << "  AMD GPU detected" << std::endl;
            
            // Check for AMDGPU driver
            if (std::filesystem::exists("/sys/module/amdgpu")) {
                std::cout << "  AMDGPU driver loaded" << std::endl;
                return true;
            }
        }
    }
    return false;
}

bool VividManager::tryXRandrCTM() {
#ifdef HAVE_X11
    if (!std::getenv("DISPLAY")) return false;
    
    Display* display = XOpenDisplay(nullptr);
    if (!display) return false;
    
    // Check for XRandR extension
    int event_base, error_base;
    if (!XRRQueryExtension(display, &event_base, &error_base)) {
        XCloseDisplay(display);
        return false;
    }
    
    // Check for CTM property support
    XRRScreenResources* screen_resources = XRRGetScreenResources(display, DefaultRootWindow(display));
    if (screen_resources && screen_resources->noutput > 0) {
        RROutput output = screen_resources->outputs[0];
        Atom ctm_atom = XInternAtom(display, "CTM", False);
        
        XRRPropertyInfo* prop_info = XRRQueryOutputProperty(display, output, ctm_atom);
        bool has_ctm = (prop_info != nullptr);
        
        if (prop_info) XFree(prop_info);
        XRRFreeScreenResources(screen_resources);
        XCloseDisplay(display);
        
        if (has_ctm) {
            std::cout << "  XRandR CTM support detected" << std::endl;
            return true;
        }
    }
    
    XCloseDisplay(display);
#endif
    return false;
}

bool VividManager::tryWaylandColorMgmt() {
    // Check if we're in a Wayland session
    if (!std::getenv("WAYLAND_DISPLAY")) return false;
    
    // This would require implementing Wayland color management protocol
    // For now, return false as it's not implemented
    return false;
}

void VividManager::detectDisplays() {
    m_displays.clear();
    
#ifdef HAVE_X11
    if (std::getenv("DISPLAY")) {
        Display* dpy = XOpenDisplay(nullptr);
        if (dpy) {
            XRRScreenResources* screen_resources = XRRGetScreenResources(dpy, DefaultRootWindow(dpy));
            if (screen_resources) {
                for (int i = 0; i < screen_resources->noutput; i++) {
                    XRROutputInfo* output_info = XRRGetOutputInfo(dpy, screen_resources, screen_resources->outputs[i]);
                    if (output_info && output_info->connection == RR_Connected) {
                        Display display;
                        display.id = std::string(output_info->name);
                        display.name = display.id;
                        display.connector = display.id;
                        display.connected = true;
                        display.currentVibrance = 0.0f;
                        m_displays.push_back(display);
                        m_baseVibrance[display.id] = 0.0f;
                    }
                    if (output_info) XRRFreeOutputInfo(output_info);
                }
                XRRFreeScreenResources(screen_resources);
            }
            XCloseDisplay(dpy);
        }
    }
#endif
    
    // If no displays found, add demo displays
    if (m_displays.empty()) {
        Display display1;
        display1.id = "DVI-D-0";
        display1.name = "DVI-D-0";
        display1.connector = "DVI-D";
        display1.connected = true;
        display1.currentVibrance = 0.0f;
        m_displays.push_back(display1);
        
        Display display2;
        display2.id = "HDMI-0";
        display2.name = "HDMI-0";
        display2.connector = "HDMI";
        display2.connected = true;
        display2.currentVibrance = 0.0f;
        m_displays.push_back(display2);
        
        m_baseVibrance["DVI-D-0"] = 0.0f;
        m_baseVibrance["HDMI-0"] = 0.0f;
    }
}

std::vector<Display> VividManager::getDisplays() {
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
            std::cout << "Demo mode: vibrance set to " << vibrance << std::endl;
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
    
    // Try to set AMD color properties via sysfs or DRM properties
    std::string command = "xrandr --output " + displayId + " --set 'saturation' " + std::to_string(saturation);
    int result = system(command.c_str());
    
    if (result != 0) {
        // Fallback to gamma-based approach
        float gamma = 1.0f / saturation;
        gamma = std::max(0.5f, std::min(2.0f, gamma));
        
        command = "xrandr --output " + displayId + " --gamma " + 
                 std::to_string(gamma) + ":" + std::to_string(gamma) + ":" + std::to_string(gamma);
        result = system(command.c_str());
    }
    
    return result == 0;
}

bool VividManager::setXRandrVibrance(const std::string& displayId, float vibrance) {
#ifdef HAVE_X11
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return false;
    
    XRRScreenResources* screen_resources = XRRGetScreenResources(dpy, DefaultRootWindow(dpy));
    if (!screen_resources) {
        XCloseDisplay(dpy);
        return false;
    }
    
    // Find the output
    RROutput target_output = None;
    for (int i = 0; i < screen_resources->noutput; i++) {
        XRROutputInfo* output_info = XRRGetOutputInfo(dpy, screen_resources, screen_resources->outputs[i]);
        if (output_info && std::string(output_info->name) == displayId) {
            target_output = screen_resources->outputs[i];
            XRRFreeOutputInfo(output_info);
            break;
        }
        if (output_info) XRRFreeOutputInfo(output_info);
    }
    
    if (target_output == None) {
        XRRFreeScreenResources(screen_resources);
        XCloseDisplay(dpy);
        return false;
    }
    
    // Create color transformation matrix for vibrance
    // This is a simplified approach - real vibrance control would need more sophisticated color math
    float saturation = 1.0f + (vibrance / 100.0f);
    saturation = std::max(0.0f, std::min(2.0f, saturation));
    
    // CTM is a 3x3 matrix in fixed-point format
    long ctm[9] = {
        (long)(saturation * 65536), 0, 0,  // Red channel
        0, (long)(saturation * 65536), 0,  // Green channel  
        0, 0, (long)(saturation * 65536)   // Blue channel
    };
    
    Atom ctm_atom = XInternAtom(dpy, "CTM", False);
    XRRChangeOutputProperty(dpy, target_output, ctm_atom, XA_INTEGER, 32,
                           PropModeReplace, (unsigned char*)ctm, 9);
    
    XRRFreeScreenResources(screen_resources);
    XCloseDisplay(dpy);
    return true;
#else
    return false;
#endif
}

bool VividManager::setWaylandVibrance(const std::string& displayId, float vibrance) {
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
            return "Demo Mode";
    }
    return "Unknown";
}

// Profile management methods would be implemented here
bool VividManager::saveProfile(const AppProfile& profile) {
    // Find existing profile or add new one
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
        // Simple JSON-like parsing (would use a proper JSON library in production)
        std::string line;
        while (std::getline(file, line)) {
            // Parse profile data
            // This is a simplified implementation
        }
    }
}

void VividManager::saveProfiles() {
    // Save profiles to config file
    std::string configPath = getConfigPath();
    std::filesystem::create_directories(std::filesystem::path(configPath).parent_path());
    
    std::ofstream file(configPath);
    if (file.is_open()) {
        // Write profiles in simple format
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
    // Start monitoring thread
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
    std::string activeWindow = getCurrentActiveWindow();
    // Check if active window matches any profile
    // Implementation would go here
}

std::string VividManager::getCurrentActiveWindow() {
#ifdef HAVE_X11
    if (std::getenv("DISPLAY")) {
        Display* dpy = XOpenDisplay(nullptr);
        if (dpy) {
            Window root = DefaultRootWindow(dpy);
            Atom active_window_atom = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
            
            Atom type;
            int format;
            unsigned long nitems, bytes_after;
            unsigned char* prop;
            
            if (XGetWindowProperty(dpy, root, active_window_atom, 0, 1, False,
                                 XA_WINDOW, &type, &format, &nitems, &bytes_after, &prop) == Success) {
                if (prop) {
                    Window active_window = *(Window*)prop;
                    XFree(prop);
                    
                    // Get window name
                    char* window_name;
                    if (XFetchName(dpy, active_window, &window_name) && window_name) {
                        std::string result(window_name);
                        XFree(window_name);
                        XCloseDisplay(dpy);
                        return result;
                    }
                }
            }
            XCloseDisplay(dpy);
        }
    }
#endif
    return "";
}
