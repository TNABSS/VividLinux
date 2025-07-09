#include <iostream>
#include <gtk/gtk.h>
#include "core/VividManager.h"
#include "ui/MainWindow.h"
#include "cli/CommandLineInterface.h"

static void activate(GtkApplication* app, gpointer user_data) {
    auto* manager = static_cast<VividManager*>(user_data);
    auto window = std::make_unique<MainWindow>(app, manager);
    window->show();
    
    // Keep window alive (this is a simplified approach)
    g_object_set_data_full(G_OBJECT(app), "window", window.release(), 
                          [](gpointer data) { delete static_cast<MainWindow*>(data); });
}

void print_simple_help() {
    std::cout << "🎮 Vivid - Digital Vibrance Control\n\n";
    std::cout << "💡 EASY COMMANDS:\n";
    std::cout << "   vivid                    Launch GUI\n";
    std::cout << "   vivid --help            Show detailed help\n";
    std::cout << "   vivid --status          Show current settings\n";
    std::cout << "   vivid --list            List your monitors\n\n";
    std::cout << "🚀 QUICK EXAMPLES:\n";
    std::cout << "   vivid --display HDMI-0 --vibrance 50    Make HDMI more colorful\n";
    std::cout << "   vivid --display eDP-1 --vibrance -30    Make laptop screen less colorful\n";
    std::cout << "   vivid --display HDMI-0 --reset          Reset HDMI to normal\n\n";
    std::cout << "📖 Vibrance values: -100 (grayscale) to +100 (super colorful)\n";
}

int main(int argc, char* argv[]) {
    // Initialize the core manager
    auto manager = std::make_unique<VividManager>();
    
    // Handle simple help first
    if (argc == 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "help")) {
        print_simple_help();
        return 0;
    }
    
    // Initialize manager
    if (!manager->initialize()) {
        std::cerr << "❌ Failed to initialize Vivid\n";
        std::cerr << "💡 This is usually fine - the GUI will still work in demo mode!\n";
    }
    
    // Handle CLI arguments
    if (argc > 1) {
        // Convert some common arguments to standard format
        std::vector<std::string> args;
        for (int i = 0; i < argc; i++) {
            std::string arg = argv[i];
            
            // Convert simple arguments
            if (arg == "--list") arg = "--list-displays";
            if (arg == "--vibrance") arg = "--set-vibrance";
            if (arg == "-s") arg = "--status";
            if (arg == "-v") arg = "--version";
            if (arg == "-h") arg = "--help";
            
            args.push_back(arg);
        }
        
        // Convert back to char* array
        std::vector<char*> c_args;
        for (auto& arg : args) {
            c_args.push_back(&arg[0]);
        }
        
        CommandLineInterface cli(manager.get());
        return cli.handleArguments(c_args.size(), c_args.data());
    }
    
    // Launch GUI
    std::cout << "🚀 Launching Vivid GUI...\n";
    
    GtkApplication* app = gtk_application_new("org.vivid.SaturationControl", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), manager.get());
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
