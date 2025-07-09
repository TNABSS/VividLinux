#include <iostream>
#include <gtk/gtk.h>
#include "ui/MainWindow.h"
#include "core/VibranceController.h"

static void activate(GtkApplication* app, gpointer user_data __attribute__((unused))) {
    auto window = std::make_unique<MainWindow>(app);
    window->show();
    
    // Keep window alive
    g_object_set_data_full(G_OBJECT(app), "window", window.release(), 
                          [](gpointer data) { delete static_cast<MainWindow*>(data); });
}

void print_help() {
    std::cout << "🎮 Vivid - Digital Vibrance Control for Linux\n\n";
    std::cout << "USAGE:\n";
    std::cout << "  vivid                                    Launch GUI\n";
    std::cout << "  vivid --list                            List displays\n";
    std::cout << "  vivid --set <display> <vibrance>        Set vibrance (-100 to +100)\n";
    std::cout << "  vivid --reset <display>                 Reset display to normal\n";
    std::cout << "  vivid --reset-all                       Reset all displays\n";
    std::cout << "  vivid --status                          Show current settings\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  vivid --set DVI-D-0 50                  Make DVI display more vibrant\n";
    std::cout << "  vivid --set HDMI-0 -30                  Make HDMI display less vibrant\n";
    std::cout << "  vivid --reset DVI-D-0                   Reset DVI to normal\n\n";
    std::cout << "VIBRANCE VALUES:\n";
    std::cout << "  -100  Completely desaturated (grayscale)\n";
    std::cout << "     0  Normal colors (default)\n";
    std::cout << "  +100  Maximum vibrance (oversaturated)\n\n";
    std::cout << "The GUI provides an easy way to adjust settings and create\n";
    std::cout << "per-application profiles for automatic vibrance switching.\n";
}

int main(int argc, char* argv[]) {
    // Handle CLI commands
    if (argc > 1) {
        std::string command = argv[1];
        
        if (command == "--help" || command == "-h") {
            print_help();
            return 0;
        }
        
        // Initialize controller for CLI operations
        VibranceController controller;
        
        if (command == "--list") {
            auto displays = controller.getDisplays();
            std::cout << "📺 Available displays:\n";
            for (const auto& display : displays) {
                std::cout << "  " << display.id << " (vibrance: " << display.currentVibrance << ")\n";
            }
            return 0;
        }
        
        if (command == "--status") {
            auto displays = controller.getDisplays();
            std::cout << "🎮 Vivid Status:\n";
            std::cout << "  Focus mode: " << (controller.getFocusMode() ? "enabled" : "disabled") << "\n";
            std::cout << "  Displays:\n";
            for (const auto& display : displays) {
                std::cout << "    " << display.id << ": " << display.currentVibrance << "\n";
            }
            
            auto profiles = controller.getProfiles();
            std::cout << "  Profiles: " << profiles.size() << " configured\n";
            return 0;
        }
        
        if (command == "--set" && argc >= 4) {
            std::string displayId = argv[2];
            int vibrance = std::stoi(argv[3]);
            
            if (controller.setVibrance(displayId, vibrance)) {
                std::cout << "✅ Set " << displayId << " vibrance to " << vibrance << "\n";
            } else {
                std::cout << "❌ Failed to set vibrance for " << displayId << "\n";
                return 1;
            }
            return 0;
        }
        
        if (command == "--reset" && argc >= 3) {
            std::string displayId = argv[2];
            
            if (controller.resetDisplay(displayId)) {
                std::cout << "✅ Reset " << displayId << " to normal\n";
            } else {
                std::cout << "❌ Failed to reset " << displayId << "\n";
                return 1;
            }
            return 0;
        }
        
        if (command == "--reset-all") {
            auto displays = controller.getDisplays();
            bool success = true;
            
            for (const auto& display : displays) {
                if (!controller.resetDisplay(display.id)) {
                    success = false;
                }
            }
            
            if (success) {
                std::cout << "✅ Reset all displays to normal\n";
            } else {
                std::cout << "❌ Failed to reset some displays\n";
                return 1;
            }
            return 0;
        }
        
        std::cout << "❌ Unknown command: " << command << "\n";
        std::cout << "Run 'vivid --help' for usage information.\n";
        return 1;
    }
    
    // Launch GUI
    std::cout << "🚀 Starting Vivid GUI...\n";
    
    GtkApplication* app = gtk_application_new("org.vivid.VibranceControl", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), nullptr);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
