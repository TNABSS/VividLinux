#pragma once

#include "../core/VividManager.h"
#include <string>

class CommandLineInterface {
public:
    explicit CommandLineInterface(VividManager* manager);
    
    int handleArguments(int argc, char* argv[]);

private:
    VividManager* m_manager;
    
    void printUsage(const std::string& programName);
    void printVersion();
    void listDisplays();
    bool setVibrance(const std::string& display, float vibrance);
    bool resetDisplay(const std::string& display);
    void printStatus();
};
