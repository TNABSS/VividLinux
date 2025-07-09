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
    std::cout << "Vivid - Digital Vibrance Control\n\n";
    std::cout << "USAGE:\n";
    std::cout << "  vivid                                    Launch GUI\n";
    std::cout << "  vivid --list                            List displays\n";
    std::cout << "  vivid --set <display> <vibrance>        Set vibrance (-100 to +100)\n";
    std::cout << "  vivid --reset <display>                 Reset display\n";
    std::cout << "  vivid --reset-all                       Reset all displays\n";
    std::cout << "  vivid --status                          Show current settings\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  vivid --set HDMI-A-1 50                 Enhance HDMI display\n";
    std::cout << "  vivid --set eDP-1 -30                   Reduce laptop display\n";
    std::cout << "  vivid --reset-all                       Reset everything\n";
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
            std::cout << "Available displays:\n";
            for (const auto& display : displays) {
                std::cout << "  " << display.id << " (vibrance: " << display.currentVibrance << ")\n";
            }
            return 0;
        }
        
        if (command == "--status") {
            auto displays = controller.getDisplays();
            std::cout << "Vivid Status:\n";
            for (const auto& display : displays) {
                std::cout << "  " << display.id << ": " << display.currentVibrance << "\n";
            }
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
            
            if (controller.setVibrance(displayId, 0)) {
                std::cout << "✅ Reset " << displayId << " to normal\n";
            } else {
                std::cout << "❌ Failed to reset " << displayId << "\n";
                return 1;
            }
            return 0;
        }
        
        if (command == "--reset-all") {
            if (controller.resetAllDisplays()) {
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
