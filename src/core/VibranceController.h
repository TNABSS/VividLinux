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

struct ProgramProfile {
    std::string name;
    std::string path;
    std::string windowTitle;
    std::map<std::string, int> displayVibrance;
    bool pathMatching = true;
    bool enabled = true;
};

class VibranceController {
public:
    VibranceController();
    ~VibranceController();
    
    // Core functionality
    bool initialize();
    std::vector<Display> getDisplays();
    bool setVibrance(const std::string& displayId, int vibrance); // -100 to +100
    int getVibrance(const std::string& displayId);
    bool resetDisplay(const std::string& displayId);
    
    // Profile management
    void saveProfile(const ProgramProfile& profile);
    void removeProfile(const std::string& name);
    std::vector<ProgramProfile> getProfiles();
    
    // Application monitoring
    void setFocusMode(bool enabled);
    bool getFocusMode() const { return m_focusMode; }
    
    // Persistence
    void saveSettings();
    void loadSettings();
    
private:
    std::vector<Display> m_displays;
    std::vector<ProgramProfile> m_profiles;
    std::map<std::string, int> m_baseVibrance; // Original values
    bool m_focusMode = false;
    bool m_initialized = false;
    
    // Implementation
    bool detectDisplays();
    bool applyVibranceXrandr(const std::string& displayId, int vibrance);
    bool applyVibranceAMD(const std::string& displayId, int vibrance);
    std::string getConfigPath();
    
    // Monitoring
    void startMonitoring();
    void stopMonitoring();
    std::string getActiveWindow();
    void checkActiveProgram();
};
