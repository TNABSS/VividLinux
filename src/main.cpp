#include <iostream>
#include <gtk/gtk.h>
#include "SaturationController.h"
#include "MainWindow.h"

// Simple command line argument handling
int handleCLI(int argc, char* argv[]) {
    SaturationController controller;
    
    if (argc == 2 && std::string(argv[1]) == "--list-displays") {
        std::cout << "Available displays:\n";
        auto displays = controller.getDisplays();
        for (const auto& display : displays) {
            float saturation = controller.getSaturation(display);
            std::cout << "  " << display << " (current: " << (int)(saturation * 100) << "%)\n";
        }
        return 0;
    }
    
    if (argc == 2 && std::string(argv[1]) == "--status") {
        std::cout << "Vivid Status:\n";
        std::cout << "  Method: " << controller.getCurrentMethod() << "\n";
        std::cout << "  Initialized: " << (controller.isInitialized() ? "Yes" : "No") << "\n";
        auto displays = controller.getDisplays();
        std::cout << "  Displays found: " << displays.size() << "\n";
        return 0;
    }
    
    if (argc == 4 && std::string(argv[1]) == "--display") {
        std::string display = argv[2];
        std::string action = argv[3];
        
        if (action == "--reset") {
            if (controller.resetSaturation(display)) {
                std::cout << "Reset " << display << " to normal saturation\n";
                return 0;
            } else {
                std::cerr << "Failed to reset " << display << "\n";
                return 1;
            }
        }
    }
    
    if (argc == 5 && std::string(argv[1]) == "--display" && std::string(argv[3]) == "--set-saturation") {
        std::string display = argv[2];
        float saturation = std::stof(argv[4]) / 100.0f; // Convert percentage to 0-2 range
        
        if (controller.setSaturation(display, saturation)) {
            std::cout << "Set " << display << " saturation to " << argv[4] << "%\n";
            return 0;
        } else {
            std::cerr << "Failed to set saturation for " << display << "\n";
            return 1;
        }
    }
    
    std::cout << "Vivid - Saturation Control for Linux\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << argv[0] << " --list-displays                           # List displays\n";
    std::cout << "  " << argv[0] << " --status                                  # Show status\n";
    std::cout << "  " << argv[0] << " --display <name> --set-saturation <0-200> # Set saturation\n";
    std::cout << "  " << argv[0] << " --display <name> --reset                  # Reset to normal\n";
    std::cout << "  " << argv[0] << "                                           # Launch GUI\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << argv[0] << " --list-displays\n";
    std::cout << "  " << argv[0] << " --display DP-1 --set-saturation 150\n";
    std::cout << "  " << argv[0] << " --display HDMI-A-1 --reset\n";
    return 1;
}

// GTK application activation callback
static void activate(GtkApplication* app, gpointer user_data) {
    MainWindow* window = new MainWindow(app);
    window->show();
}

int main(int argc, char* argv[]) {
    std::cout << "Starting Vivid v1.0.0...\n";
    
    // Handle command line arguments
    if (argc > 1) {
        return handleCLI(argc, argv);
    }
    
    // Launch GUI application
    std::cout << "Launching GUI interface...\n";
    GtkApplication* app = gtk_application_new("org.vivid.SaturationControl", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), nullptr);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
