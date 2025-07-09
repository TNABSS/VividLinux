#pragma once
#include <string>
#include <vector>
#include <map>

// Simple class to handle saturation control across different methods
class SaturationController {
public:
    SaturationController();
    ~SaturationController();
    
    // Core functionality
    bool initialize();
    std::vector<std::string> getDisplays();
    bool setSaturation(const std::string& display, float saturation); // 0.0 to 2.0
    float getSaturation(const std::string& display);
    bool resetSaturation(const std::string& display);
    
    // Get current method being used
    std::string getCurrentMethod() const { return currentMethod; }
    bool isInitialized() const { return initialized; }
    
private:
    std::string currentMethod;
    std::map<std::string, float> currentSaturations;
    bool initialized;
    
    // Different methods to try (in order of preference)
    bool tryX11Method();
    bool tryDDCMethod();
    
    // Helper functions
    bool isX11Session();
    std::vector<std::string> getX11Displays();
    bool setX11Saturation(const std::string& display, float saturation);
};
