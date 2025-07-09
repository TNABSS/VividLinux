#include <iostream>
#include <gtk/gtk.h>
#include "core/VividManager.h"
#include "ui/MainWindow.h"
#include "cli/CommandLineInterface.h"

static void activate(GtkApplication* app, gpointer user_data) {
    auto* manager = static_cast<VividManager*>(user_data);
    auto window = std::make_unique<MainWindow>(app, manager);
    window->show();
}

int main(int argc, char* argv[]) {
    // Initialize the core manager
    auto manager = std::make_unique<VividManager>();
    if (!manager->initialize()) {
        std::cerr << "Failed to initialize Vivid manager" << std::endl;
        return 1;
    }
    
    // Handle CLI arguments
    if (argc > 1) {
        CommandLineInterface cli(manager.get());
        return cli.handleArguments(argc, argv);
    }
    
    // Launch GUI
    GtkApplication* app = gtk_application_new("org.vivid.SaturationControl", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), manager.get());
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
