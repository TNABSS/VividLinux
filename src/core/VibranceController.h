#pragma once
#include <string>
#include <vector>
#include <map>

struct Display {
    std::string id;
    std::string name;
    int currentVibrance = 0; // -100 to +100
    bool connected = true;
};

class VibranceController {
public:
    VibranceController();
    ~VibranceController();
    
    // Core functionality
    bool initialize();
    std::vector<Display> getDisplays();
    bool setVibrance(const std::string& displayId, int vibrance); // Real-time application
    int getVibrance(const std::string& displayId);
    bool resetAllDisplays();
    
    // System management
    bool installSystemWide();
    bool isSystemInstalled();
    
    // Auto-setup
    bool autoInstallDependencies();
    bool isReady() const { return m_initialized; }
    
private:
    std::vector<Display> m_displays;
    std::map<std::string, int> m_currentVibrance;
    bool m_initialized = false;
    
    // Implementation
    bool detectDisplays();
    bool applyVibranceImmediate(const std::string& displayId, int vibrance);
    bool checkDependencies();
    bool installMissingDeps();
};
