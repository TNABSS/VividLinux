#include "SaturationController.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef HAVE_X11
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#endif

SaturationController::SaturationController() : currentMethod("None"), initialized(false) {
    initialize();
}

SaturationController::~SaturationController() = default;

bool SaturationController::initialize() {
    std::cout << "Initializing Vivid saturation controller...\n";
    
    // Try different methods in order of preference
    if (tryX11Method()) {
        currentMethod = "X11/xrandr";
        std::cout << "✓ Using X11/xrandr method\n";
        initialized = true;
        return true;
    }
    
    if (tryDDCMethod()) {
        currentMethod = "DDC/CI";
        std::cout << "✓ Using DDC/CI method\n";
        initialized = true;
        return true;
    }
    
    std::cerr << "✗ No compatible saturation control method found!\n";
    std::cerr << "  Make sure you're running on X11 or have ddcutil installed.\n";
    std::cerr << "  Current session: ";
    if (std::getenv("WAYLAND_DISPLAY")) {
        std::cerr << "Wayland (limited support)\n";
    } else if (std::getenv("DISPLAY")) {
        std::cerr << "X11 (should work)\n";
    } else {
        std::cerr << "Unknown\n";
    }
    
    currentMethod = "None (Demo Mode)";
    initialized = false;
    return false;
}

bool SaturationController::isX11Session() {
    return std::getenv("DISPLAY") != nullptr;
}

bool SaturationController::tryX11Method() {
    if (!isX11Session()) {
        std::cout << "  X11 not available (not running X11 session)\n";
        return false;
    }
    
#ifdef HAVE_X11
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cout << "  X11 not available (cannot connect to display)\n";
        return false;
    }
    
    // Check if xrandr extension is available
    int event_base, error_base;
    if (!XRRQueryExtension(display, &event_base, &error_base)) {
        std::cout << "  X11 not available (xrandr extension missing)\n";
        XCloseDisplay(display);
        return false;
    }
    
    XCloseDisplay(display);
    std::cout << "  X11/xrandr available\n";
    return true;
#else
    std::cout << "  X11 not available (not compiled with X11 support)\n";
    return false;
#endif
}

bool SaturationController::tryDDCMethod() {
    // Check if ddcutil is available
    std::cout << "  Checking for ddcutil...\n";
    int result = system("which ddcutil > /dev/null 2>&1");
    if (result == 0) {
        std::cout << "  ddcutil found\n";
        return true;
    } else {
        std::cout << "  ddcutil not found (install with: sudo dnf install ddcutil)\n";
        return false;
    }
}

std::vector<std::string> SaturationController::getDisplays() {
    if (currentMethod == "X11/xrandr") {
        return getX11Displays();
    } else if (currentMethod == "DDC/CI") {
        // Use ddcutil to get displays
        std::vector<std::string> displays;
        std::cout << "Detecting DDC/CI displays...\n";
        FILE* pipe = popen("ddcutil detect --brief 2>/dev/null | grep 'Display' | awk '{print $2}'", "r");
        if (pipe) {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe)) {
                std::string display = buffer;
                display.erase(display.find_last_not_of(" \n\r\t") + 1);
                if (!display.empty()) {
                    displays.push_back("DDC-" + display);
                }
            }
            pclose(pipe);
        }
        if (displays.empty()) {
            displays.push_back("No DDC/CI displays found");
        }
        return displays;
    }
    
    // Demo mode - return fake displays
    return {"Demo-Display-1", "Demo-Display-2"};
}

std::vector<std::string> SaturationController::getX11Displays() {
    std::vector<std::string> displays;
    
#ifdef HAVE_X11
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        displays.push_back("Error: Cannot connect to X11");
        return displays;
    }
    
    XRRScreenResources* screen_resources = XRRGetScreenResources(dpy, DefaultRootWindow(dpy));
    if (!screen_resources) {
        displays.push_back("Error: Cannot get screen resources");
        XCloseDisplay(dpy);
        return displays;
    }
    
    std::cout << "Found " << screen_resources->noutput << " X11 outputs\n";
    
    for (int i = 0; i < screen_resources->noutput; i++) {
        XRROutputInfo* output_info = XRRGetOutputInfo(dpy, screen_resources, screen_resources->outputs[i]);
        if (output_info && output_info->connection == RR_Connected) {
            std::string displayName = std::string(output_info->name);
            displays.push_back(displayName);
            std::cout << "  Found connected display: " << displayName << "\n";
        }
        if (output_info) XRRFreeOutputInfo(output_info);
    }
    
    XRRFreeScreenResources(screen_resources);
    XCloseDisplay(dpy);
#else
    displays.push_back("X11 support not compiled");
#endif
    
    if (displays.empty()) {
        displays.push_back("No displays found");
    }
    
    return displays;
}

bool SaturationController::setSaturation(const std::string& display, float saturation) {
    // Clamp saturation to valid range
    saturation = std::max(0.0f, std::min(2.0f, saturation));
    
    std::cout << "Setting " << display << " saturation to " << (saturation * 100) << "%\n";
    
    if (currentMethod == "X11/xrandr") {
        bool success = setX11Saturation(display, saturation);
        if (success) {
            currentSaturations[display] = saturation;
            std::cout << "✓ Successfully set saturation\n";
        } else {
            std::cout << "✗ Failed to set saturation\n";
        }
        return success;
    } else if (currentMethod == "DDC/CI") {
        // Use ddcutil to set saturation (VCP code 0x8A is common for saturation)
        if (display.substr(0, 4) == "DDC-") {
            std::string displayNum = display.substr(4); // Remove "DDC-" prefix
            int vcpValue = static_cast<int>(saturation * 50); // Convert to 0-100 range
            
            std::string command = "ddcutil setvcp 8A " + std::to_string(vcpValue) + " --display " + displayNum + " 2>/dev/null";
            std::cout << "Running: " << command << "\n";
            int result = system(command.c_str());
            
            if (result == 0) {
                currentSaturations[display] = saturation;
                std::cout << "✓ Successfully set DDC/CI saturation\n";
                return true;
            } else {
                std::cout << "✗ DDC/CI command failed\n";
            }
        }
    } else {
        // Demo mode
        currentSaturations[display] = saturation;
        std::cout << "✓ Demo mode: saturation set (no actual change)\n";
        return true;
    }
    
    return false;
}

bool SaturationController::setX11Saturation(const std::string& display, float saturation) {
    // Use xrandr to adjust gamma as a saturation approximation
    // This is a simplified approach - real saturation would need color matrix manipulation
    
    float gamma = 1.0f / saturation; // Inverse relationship for visual effect
    gamma = std::max(0.5f, std::min(2.0f, gamma));
    
    std::string command = "xrandr --output " + display + " --gamma " + 
                         std::to_string(gamma) + ":" + std::to_string(gamma) + ":" + std::to_string(gamma);
    
    std::cout << "Running: " << command << "\n";
    int result = system(command.c_str());
    
    return result == 0;
}

float SaturationController::getSaturation(const std::string& display) {
    auto it = currentSaturations.find(display);
    return (it != currentSaturations.end()) ? it->second : 1.0f;
}

bool SaturationController::resetSaturation(const std::string& display) {
    std::cout << "Resetting " << display << " to normal saturation\n";
    return setSaturation(display, 1.0f);
}
