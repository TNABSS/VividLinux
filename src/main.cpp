#include <iostream>
#include <gtk/gtk.h>
#include "ui/MainWindow.h"
#include "core/VibranceController.h"

static void activate(GtkApplication* app, gpointer user_data) {
    auto window = std::make_unique<MainWindow>(app);
    window->show();
    
    g_object_set_data_full(G_OBJECT(app), "window", window.release(), 
                          [](gpointer data) { delete static_cast<MainWindow*>(data); });
}

void print_help() {
    std::cout << "Vivid - Digital Vibrance Control\n\n";
    std::cout << "USAGE:\n";
    std::cout << "  vivid                                    Launch GUI\n";
    std::cout << "  vivid --list                            List displays\n";
    std::cout << "  vivid --set <display> <value>           Set vibrance (-100 to 100)\n";
    std::cout << "  vivid --reset                           Reset all displays\n";
    std::cout << "  vivid --help                            Show this help\n\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "  vivid --set HDMI-A-1 50                 Set HDMI display to 50\n";
    std::cout << "  vivid --reset                           Reset all to 0\n";
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string command = argv[1];
        
        if (command == "--help" || command == "-h") {
            print_help();
            return 0;
        }
        
        VibranceController controller;
        
        if (command == "--list") {
            auto displays = controller.getDisplays();
            for (const auto& display : displays) {
                std::cout << display.id << " (" << display.currentVibrance << ")\n";
            }
            return 0;
        }
        
        if (command == "--set" && argc >= 4) {
            std::string displayId = argv[2];
            int vibrance = std::stoi(argv[3]);
            controller.setVibrance(displayId, vibrance);
            return 0;
        }
        
        if (command == "--reset") {
            controller.resetAllDisplays();
            return 0;
        }
        
        std::cout << "Unknown command. Use --help for usage.\n";
        return 1;
    }
    
    GtkApplication* app = gtk_application_new("org.vivid.VibranceControl", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), nullptr);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
