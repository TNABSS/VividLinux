#pragma once

#include <string>
#include <vector>

class AutostartManager {
public:
    AutostartManager();
    ~AutostartManager();
    
    // Core autostart functionality
    bool isEnabled();
    bool enable();
    bool disable();
    
    // Status and debugging
    std::string getStatus();
    std::vector<std::string> getDebugInfo();
    
    // Configuration
    void setMinimizeToTray(bool minimize) { m_minimizeToTray = minimize; }
    void setStartWithProfiles(bool start) { m_startWithProfiles = start; }
    void setDelayedStart(int seconds) { m_delayedStart = seconds; }
    
private:
    bool m_minimizeToTray;
    bool m_startWithProfiles;
    int m_delayedStart;
    
    // Helper methods
    std::string getAutostartDirectory();
    std::string getAutostartFilePath();
    std::string getDesktopFileContent();
    std::string getExecutablePath();
    bool createAutostartDirectory();
    bool writeDesktopFile(const std::string& content);
    bool removeDesktopFile();
    
    // Validation and debugging
    bool validateDesktopFile();
    bool testAutostartFile();
    std::vector<std::string> collectDebugInfo();
};
