#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

// Forward declaration
class AutostartManager;

struct VividDisplay {
    std::string id;
    std::string name;
    std::string connector;
    bool connected;
    float currentVibrance;
};

struct AppProfile {
    std::string name;
    std::string executable;
    std::string windowTitle;
    std::map<std::string, float> displayVibrance;
    bool pathMatching;
    bool enabled;
};

enum class VibranceMethod {
    AMD_COLOR_PROPERTIES,
    XRANDR_CTM,
    WAYLAND_COLOR_MGMT,
    DEMO_MODE
};

class VividManager {
public:
    VividManager();
    ~VividManager();
    
    // Core functionality
    bool initialize();
    std::vector<VividDisplay> getDisplays();
    bool setVibrance(const std::string& displayId, float vibrance); // -100 to +100
    bool setVibranceSafe(const std::string& displayId, float vibrance); // Safe version with limits
    float getVibrance(const std::string& displayId);
    bool resetVibrance(const std::string& displayId);
    
    // Profile management
    bool saveProfile(const AppProfile& profile);
    bool deleteProfile(const std::string& name);
    std::vector<AppProfile> getProfiles();
    AppProfile* findProfile(const std::string& name);
    
    // Application monitoring
    void startApplicationMonitoring();
    void stopApplicationMonitoring();
    bool isMonitoringEnabled() const { return m_monitoringEnabled; }
    void setMonitoringEnabled(bool enabled);

    // Autostart functionality
    bool isAutostartEnabled();
    bool enableAutostart();
    bool disableAutostart();
    std::string getAutostartStatus();
    
    // Method info
    VibranceMethod getCurrentMethod() const { return m_currentMethod; }
    std::string getMethodName() const;
    bool isInitialized() const { return m_initialized; }

private:
    VibranceMethod m_currentMethod;
    bool m_initialized;
    bool m_monitoringEnabled;
    std::vector<VividDisplay> m_displays;
    std::vector<AppProfile> m_profiles;
    std::map<std::string, float> m_baseVibrance;
    std::map<std::string, float> m_originalVibrance; // Store original values for safety
    
    // Autostart manager
    std::unique_ptr<AutostartManager> m_autostartManager;
    
    // Detection methods
    bool tryAMDColorProperties();
    bool tryAMDXrandrFallback();
    bool tryXRandrCTM();
    bool tryWaylandColorMgmt();
    
    // Implementation methods
    bool setAMDVibrance(const std::string& displayId, float vibrance);
    bool setXRandrVibrance(const std::string& displayId, float vibrance);
    bool setWaylandVibrance(const std::string& displayId, float vibrance);
    
    // Helper methods
    void detectDisplays();
    void loadProfiles();
    void saveProfiles();
    std::string getConfigPath();
    
    // Application monitoring
    void checkActiveApplication();
    std::string getCurrentActiveWindow();
    void applyProfileForApp(const std::string& appName, const std::string& windowTitle);
};
