#pragma once
#include <string>
#include <vector>
#include <map>

struct Display {
    std::string id;
    std::string name;
    int currentVibrance = 0;
    bool connected = true;
};

class VibranceController {
public:
    VibranceController();
    ~VibranceController();
    
    bool initialize();
    std::vector<Display> getDisplays();
    bool setVibrance(const std::string& displayId, int vibrance);
    int getVibrance(const std::string& displayId);
    bool resetAllDisplays();
    bool installSystemWide();
    bool isSystemInstalled();
    bool isReady() const { return m_initialized; }
    
private:
    std::vector<Display> m_displays;
    std::map<std::string, int> m_currentVibrance;
    bool m_initialized = false;
    
    bool detectDisplays();
    bool applyVibranceImmediate(const std::string& displayId, int vibrance);
    bool applyXGamma(const std::string& displayId, int vibrance);
    bool applyRedshift(int vibrance);
    bool applyXCalib(const std::string& displayId, int vibrance);
    bool applyXRandr(const std::string& displayId, int vibrance);
};
