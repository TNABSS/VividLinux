#include "CommandLineInterface.h"
#include <iostream>
#include <iomanip>

CommandLineInterface::CommandLineInterface(VividManager* manager) 
    : m_manager(manager) {}

int CommandLineInterface::handleArguments(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    
    if (command == "--help" || command == "-h") {
        printUsage(argv[0]);
        return 0;
    }
    
    if (command == "--version" || command == "-v") {
        printVersion();
        return 0;
    }
    
    if (command == "--list-displays" || command == "-l") {
        listDisplays();
        return 0;
    }
    
    if (command == "--status" || command == "-s") {
        printStatus();
        return 0;
    }
    
    if (command == "--display" || command == "-d") {
        if (argc < 4) {
            std::cerr << "Error: --display requires display name and action" << std::endl;
            return 1;
        }
        
        std::string displayId = argv[2];
        std::string action = argv[3];
        
        if (action == "--set-vibrance") {
            if (argc < 5) {
                std::cerr << "Error: --set-vibrance requires a value (-100 to +100)" << std::endl;
                return 1;
            }
            
            float vibrance = std::stof(argv[4]);
            return setVibrance(displayId, vibrance) ? 0 : 1;
        }
        
        if (action == "--reset") {
            return resetDisplay(displayId) ? 0 : 1;
        }
        
        std::cerr << "Error: Unknown action '" << action << "'" << std::endl;
        return 1;
    }
    
    std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
    printUsage(argv[0]);
    return 1;
}

void CommandLineInterface::printUsage(const std::string& programName) {
    std::cout << "Vivid - Digital Vibrance Control for Linux\n\n";
    std::cout << "Usage: " << programName << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                    Show this help message\n";
    std::cout << "  -v, --version                 Show version information\n";
    std::cout << "  -l, --list-displays           List available displays\n";
    std::cout << "  -s, --status                  Show current status and method\n";
    std::cout << "  -d, --display <name> <action> Control specific display\n\n";
    std::cout << "Display Actions:\n";
    std::cout << "  --set-vibrance <value>        Set vibrance (-100 to +100)\n";
    std::cout << "  --reset                       Reset to default vibrance\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " --list-displays\n";
    std::cout << "  " << programName << " --display DVI-D-0 --set-vibrance 50\n";
    std::cout << "  " << programName << " --display HDMI-0 --reset\n";
}

void CommandLineInterface::printVersion() {
    std::cout << "Vivid v1.0.0 - Digital Vibrance Control for Linux\n";
    std::cout << "Method: " << m_manager->getMethodName() << std::endl;
}

void CommandLineInterface::listDisplays() {
    auto displays = m_manager->getDisplays();
    
    if (displays.empty()) {
        std::cout << "No displays found." << std::endl;
        return;
    }
    
    std::cout << "Available displays:" << std::endl;
    for (const auto& display : displays) {
        std::cout << "  " << display.id << " (" << display.name << ") - " 
                  << std::fixed << std::setprecision(0) 
                  << display.currentVibrance << std::endl;
    }
}

void CommandLineInterface::printStatus() {
    std::cout << "Vivid Status:" << std::endl;
    std::cout << "  Method: " << m_manager->getMethodName() << std::endl;
    std::cout << "  Initialized: " << (m_manager->isInitialized() ? "Yes" : "No") << std::endl;
    
    auto displays = m_manager->getDisplays();
    std::cout << "  Displays: " << displays.size() << " found" << std::endl;
    
    if (!displays.empty()) {
        std::cout << "  Current vibrance settings:" << std::endl;
        for (const auto& display : displays) {
            std::cout << "    " << display.id << ": " 
                      << std::fixed << std::setprecision(0) 
                      << display.currentVibrance << std::endl;
        }
    }
}

bool CommandLineInterface::setVibrance(const std::string& display, float vibrance) {
    if (m_manager->setVibrance(display, vibrance)) {
        std::cout << "Set " << display << " vibrance to " << vibrance << std::endl;
        return true;
    } else {
        std::cerr << "Failed to set vibrance for " << display << std::endl;
        return false;
    }
}

bool CommandLineInterface::resetDisplay(const std::string& display) {
    if (m_manager->resetVibrance(display)) {
        std::cout << "Reset " << display << " to default vibrance" << std::endl;
        return true;
    } else {
        std::cerr << "Failed to reset " << display << std::endl;
        return false;
    }
}
