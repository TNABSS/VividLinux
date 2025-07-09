#include "MainWindow.h"
#include <iostream>
#include <iomanip>
#include <sstream>

MainWindow::MainWindow(GtkApplication* app, VividManager* manager) 
    : m_manager(manager) {
    
    m_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(m_window), "Vivid - Digital Vibrance Control");
    gtk_window_set_default_size(GTK_WINDOW(m_window), 500, 400);
    gtk_window_set_resizable(GTK_WINDOW(m_window), TRUE);
    
    applyDarkTheme();
    setupUI();
    updateVibranceControls();
    updateProgramList();
}

MainWindow::~MainWindow() = default;

void MainWindow::applyDarkTheme() {
    GtkCssProvider* provider = gtk_css_provider_new();
    
    const char* css = R"(
        window {
            background-color: #2b2b2b;
            color: #ffffff;
        }
        
        .dark-frame {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-radius: 6px;
            padding: 10px;
        }
        
        .display-tab {
            background-color: #404040;
            border: 1px solid #555555;
            padding: 8px 16px;
            margin: 2px;
        }
        
        .display-tab:checked {
            background-color: #0078d4;
        }
        
        .vibrance-scale {
            background-color: #404040;
        }
        
        .vibrance-scale trough {
            background-color: #555555;
            border-radius: 3px;
        }
        
        .vibrance-scale slider {
            background-color: #0078d4;
            border: 2px solid #ffffff;
            border-radius: 50%;
            min-width: 16px;
            min-height: 16px;
        }
        
        .program-list {
            background-color: #2b2b2b;
            border: 1px solid #555555;
        }
        
        .add-button {
            background-color: #0078d4;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
        }
        
        .remove-button {
            background-color: #666666;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
        }
    )";
    
    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    
    g_object_unref(provider);
}

void MainWindow::setupUI() {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(m_window), vbox);
    
    // Menu bar
    setupMenuBar();
    
    // Main content area
    GtkWidget* mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(mainBox, 15);
    gtk_widget_set_margin_end(mainBox, 15);
    gtk_widget_set_margin_top(mainBox, 10);
    gtk_widget_set_margin_bottom(mainBox, 15);
    gtk_box_append(GTK_BOX(vbox), mainBox);
    
    // Focus checkbox
    m_focusCheckbox = gtk_check_button_new_with_label("Set vibrance only when program is in focus");
    gtk_widget_set_margin_bottom(m_focusCheckbox, 10);
    g_signal_connect(m_focusCheckbox, "toggled", G_CALLBACK(onFocusCheckboxToggled), this);
    gtk_box_append(GTK_BOX(mainBox), m_focusCheckbox);
    
    // Display tabs
    setupDisplayTabs();
    gtk_box_append(GTK_BOX(mainBox), m_notebook);
    
    // Program management section
    GtkWidget* programFrame = gtk_frame_new(nullptr);
    gtk_widget_add_css_class(programFrame, "dark-frame");
    gtk_widget_set_margin_top(programFrame, 10);
    gtk_box_append(GTK_BOX(mainBox), programFrame);
    
    GtkWidget* programBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(programBox, 10);
    gtk_widget_set_margin_end(programBox, 10);
    gtk_widget_set_margin_top(programBox, 10);
    gtk_widget_set_margin_bottom(programBox, 10);
    gtk_frame_set_child(GTK_FRAME(programFrame), programBox);
    
    // Program buttons
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_append(GTK_BOX(programBox), buttonBox);
    
    m_addProgramButton = gtk_button_new_with_label("Add program");
    gtk_widget_add_css_class(m_addProgramButton, "add-button");
    g_signal_connect(m_addProgramButton, "clicked", G_CALLBACK(onAddProgramClicked), this);
    gtk_box_append(GTK_BOX(buttonBox), m_addProgramButton);
    
    m_removeProgramButton = gtk_button_new_with_label("Remove program");
    gtk_widget_add_css_class(m_removeProgramButton, "remove-button");
    g_signal_connect(m_removeProgramButton, "clicked", G_CALLBACK(onRemoveProgramClicked), this);
    gtk_box_append(GTK_BOX(buttonBox), m_removeProgramButton);
    
    // Program list
    setupProgramList();
    gtk_box_append(GTK_BOX(programBox), m_programList);
    
    // Status bar
    m_statusLabel = gtk_label_new("");
    gtk_widget_set_halign(m_statusLabel, GTK_ALIGN_START);
    gtk_widget_set_margin_top(m_statusLabel, 10);
    std::string status = "Method: " + m_manager->getMethodName();
    gtk_label_set_text(GTK_LABEL(m_statusLabel), status.c_str());
    gtk_box_append(GTK_BOX(mainBox), m_statusLabel);
}

void MainWindow::setupMenuBar() {
    // Create a simple header bar instead of deprecated menu bar
    GtkWidget* headerBar = gtk_header_bar_new();
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(headerBar), gtk_label_new("Vivid"));
    gtk_window_set_titlebar(GTK_WINDOW(m_window), headerBar);
    
    // Add menu button
    GtkWidget* menuButton = gtk_menu_button_new();
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerBar), menuButton);
    
    // Create menu model
    GMenu* menu = g_menu_new();
    g_menu_append(menu, "About", "app.about");
    g_menu_append(menu, "Quit", "app.quit");
    
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menuButton), G_MENU_MODEL(menu));
    g_object_unref(menu);
}

void MainWindow::setupDisplayTabs() {
    m_notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(m_notebook), GTK_POS_TOP);
    
    auto displays = m_manager->getDisplays();
    for (const auto& display : displays) {
        // Create tab content
        GtkWidget* tabContent = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
        gtk_widget_set_margin_start(tabContent, 20);
        gtk_widget_set_margin_end(tabContent, 20);
        gtk_widget_set_margin_top(tabContent, 20);
        gtk_widget_set_margin_bottom(tabContent, 20);
        
        // Vibrance label
        std::string labelText = "Vibrance for " + display.name;
        GtkWidget* vibranceLabel = gtk_label_new(labelText.c_str());
        gtk_widget_set_halign(vibranceLabel, GTK_ALIGN_START);
        gtk_widget_set_margin_bottom(vibranceLabel, 10);
        gtk_box_append(GTK_BOX(tabContent), vibranceLabel);
        
        // Vibrance control container
        GtkWidget* controlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_box_append(GTK_BOX(tabContent), controlBox);
        
        // Vibrance scale
        GtkWidget* vibranceScale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -100.0, 100.0, 1.0);
        gtk_widget_add_css_class(vibranceScale, "vibrance-scale");
        gtk_scale_set_value_pos(GTK_SCALE(vibranceScale), GTK_POS_RIGHT);
        gtk_scale_set_digits(GTK_SCALE(vibranceScale), 0);
        gtk_range_set_value(GTK_RANGE(vibranceScale), display.currentVibrance);
        gtk_widget_set_hexpand(vibranceScale, TRUE);
        
        // Add scale marks
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), -100.0, GTK_POS_BOTTOM, nullptr);
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), -50.0, GTK_POS_BOTTOM, nullptr);
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), 0.0, GTK_POS_BOTTOM, nullptr);
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), 50.0, GTK_POS_BOTTOM, nullptr);
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), 100.0, GTK_POS_BOTTOM, nullptr);
        
        g_signal_connect(vibranceScale, "value-changed", G_CALLBACK(onVibranceChanged), this);
        gtk_box_append(GTK_BOX(controlBox), vibranceScale);
        
        // Vibrance spin button
        GtkWidget* vibranceSpin = gtk_spin_button_new_with_range(-100.0, 100.0, 1.0);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(vibranceSpin), display.currentVibrance);
        g_signal_connect(vibranceSpin, "value-changed", G_CALLBACK(onVibranceSpinChanged), this);
        gtk_box_append(GTK_BOX(controlBox), vibranceSpin);
        
        // Store references
        m_vibranceScales[display.id] = vibranceScale;
        m_vibranceSpins[display.id] = vibranceSpin;
        
        // Create tab label
        GtkWidget* tabLabel = gtk_label_new(display.id.c_str());
        
        // Add tab to notebook
        gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), tabContent, tabLabel);
        m_displayTabs[display.id] = tabContent;
    }
    
    g_signal_connect(m_notebook, "switch-page", G_CALLBACK(onDisplayTabChanged), this);
}

void MainWindow::setupProgramList() {
    // Create scrolled window for the list
    GtkWidget* scrolled = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrolled, -1, 150);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    // Use GtkListBox instead of deprecated GtkTreeView
    m_programList = gtk_list_box_new();
    gtk_widget_add_css_class(m_programList, "program-list");
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), m_programList);
    m_programList = scrolled; // Store the scrolled window as the main widget
}

void MainWindow::onVibranceChanged(GtkRange* range, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    // Find which display this scale belongs to
    for (const auto& pair : window->m_vibranceScales) {
        if (pair.second == GTK_WIDGET(range)) {
            double value = gtk_range_get_value(range);
            window->m_manager->setVibrance(pair.first, static_cast<float>(value));
            
            // Update corresponding spin button
            auto spinIt = window->m_vibranceSpins.find(pair.first);
            if (spinIt != window->m_vibranceSpins.end()) {
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinIt->second), value);
            }
            break;
        }
    }
}

void MainWindow::onVibranceSpinChanged(GtkSpinButton* spin, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    // Find which display this spin belongs to
    for (const auto& pair : window->m_vibranceSpins) {
        if (pair.second == GTK_WIDGET(spin)) {
            double value = gtk_spin_button_get_value(spin);
            window->m_manager->setVibrance(pair.first, static_cast<float>(value));
            
            // Update corresponding scale
            auto scaleIt = window->m_vibranceScales.find(pair.first);
            if (scaleIt != window->m_vibranceScales.end()) {
                gtk_range_set_value(GTK_RANGE(scaleIt->second), value);
            }
            break;
        }
    }
}

void MainWindow::onDisplayTabChanged(GtkNotebook* notebook __attribute__((unused)), GtkWidget* page __attribute__((unused)), guint page_num, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    // Update current display based on tab selection
    auto displays = window->m_manager->getDisplays();
    if (page_num < displays.size()) {
        window->m_currentDisplay = displays[page_num].id;
    }
}

void MainWindow::onFocusCheckboxToggled(GtkCheckButton* button, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    bool enabled = gtk_check_button_get_active(button);
    window->m_manager->setMonitoringEnabled(enabled);
}

void MainWindow::onAddProgramClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    window->showAddProgramDialog();
}

void MainWindow::onRemoveProgramClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    // Get selected program from list box
    GtkListBox* listBox = GTK_LIST_BOX(gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(window->m_programList)));
    GtkListBoxRow* selected = gtk_list_box_get_selected_row(listBox);
    
    if (selected) {
        // Remove the selected program
        // This is a simplified implementation
        gtk_list_box_remove(listBox, GTK_WIDGET(selected));
    }
}

void MainWindow::onProgramListSelectionChanged(GtkTreeSelection* selection __attribute__((unused)), gpointer user_data __attribute__((unused))) {
    // Handle program selection change
}

void MainWindow::showAddProgramDialog() {
    // Use GtkWindow instead of deprecated GtkDialog
    GtkWidget* dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Add Program");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(m_window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);
    
    // Create content
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_window_set_child(GTK_WINDOW(dialog), vbox);
    
    // Add content
    GtkWidget* label = gtk_label_new("Select a program to add:");
    gtk_box_append(GTK_BOX(vbox), label);
    
    GtkWidget* entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Program name or path");
    gtk_box_append(GTK_BOX(vbox), entry);
    
    // Button box
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(buttonBox, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vbox), buttonBox);
    
    GtkWidget* cancelButton = gtk_button_new_with_label("Cancel");
    GtkWidget* okButton = gtk_button_new_with_label("OK");
    gtk_widget_add_css_class(okButton, "suggested-action");
    
    gtk_box_append(GTK_BOX(buttonBox), cancelButton);
    gtk_box_append(GTK_BOX(buttonBox), okButton);
    
    // Connect signals
    g_signal_connect_swapped(cancelButton, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    g_signal_connect_swapped(okButton, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    
    gtk_window_present(GTK_WINDOW(dialog));
}

void MainWindow::updateVibranceControls() {
    auto displays = m_manager->getDisplays();
    for (const auto& display : displays) {
        auto scaleIt = m_vibranceScales.find(display.id);
        auto spinIt = m_vibranceSpins.find(display.id);
        
        if (scaleIt != m_vibranceScales.end()) {
            gtk_range_set_value(GTK_RANGE(scaleIt->second), display.currentVibrance);
        }
        if (spinIt != m_vibranceSpins.end()) {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinIt->second), display.currentVibrance);
        }
    }
}

void MainWindow::updateProgramList() {
    GtkListBox* listBox = GTK_LIST_BOX(gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(m_programList)));
    
    // Clear existing items
    GtkWidget* child;
    while ((child = gtk_widget_get_first_child(GTK_WIDGET(listBox))) != nullptr) {
        gtk_list_box_remove(listBox, child);
    }
    
    // Add profiles
    auto profiles = m_manager->getProfiles();
    for (const auto& profile : profiles) {
        GtkWidget* label = gtk_label_new(profile.name.c_str());
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_list_box_append(listBox, label);
    }
}

void MainWindow::show() {
    gtk_window_present(GTK_WINDOW(m_window));
}
