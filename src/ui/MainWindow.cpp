#include "MainWindow.h"
#include <iostream>
#include <filesystem>

MainWindow::MainWindow(GtkApplication* app) {
    m_controller = std::make_unique<VibranceController>();
    
    m_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(m_window), "Vivid");
    gtk_window_set_default_size(GTK_WINDOW(m_window), 450, 350);
    gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);
    
    applyDarkTheme();
    setupUI();
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
        
        .main-container {
            background-color: #2b2b2b;
            padding: 8px;
        }
        
        notebook {
            background-color: #3c3c3c;
            border: 1px solid #555555;
        }
        
        notebook tab {
            background-color: #404040;
            color: #cccccc;
            border: 1px solid #555555;
            padding: 8px 16px;
            margin: 2px;
        }
        
        notebook tab:checked {
            background-color: #2b2b2b;
            color: #ffffff;
        }
        
        .vibrance-container {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 12px;
            margin: 8px 0;
        }
        
        scale {
            margin: 8px 0;
        }
        
        scale trough {
            background: linear-gradient(to right, #666666 0%, #4a9eff 50%, #666666 100%);
            border-radius: 3px;
            min-height: 6px;
        }
        
        scale slider {
            background-color: #ffffff;
            border: 2px solid #4a9eff;
            border-radius: 50%;
            min-width: 16px;
            min-height: 16px;
        }
        
        spinbutton {
            background-color: #404040;
            color: #ffffff;
            border: 1px solid #666666;
            border-radius: 3px;
            min-width: 60px;
        }
        
        button {
            background-color: #4a9eff;
            color: #ffffff;
            border: none;
            border-radius: 3px;
            padding: 8px 16px;
            margin: 4px;
        }
        
        button:hover {
            background-color: #357abd;
        }
        
        .remove-button {
            background-color: #666666;
        }
        
        .remove-button:hover {
            background-color: #555555;
        }
        
        checkbutton {
            color: #ffffff;
        }
        
        checkbutton check {
            background-color: #404040;
            border: 1px solid #666666;
        }
        
        checkbutton check:checked {
            background-color: #4a9eff;
            border-color: #4a9eff;
        }
        
        listbox {
            background-color: #404040;
            border: 1px solid #666666;
            border-radius: 3px;
        }
        
        listbox row {
            background-color: transparent;
            color: #ffffff;
            padding: 8px;
            border-bottom: 1px solid #555555;
        }
        
        listbox row:hover {
            background-color: #4a4a4a;
        }
        
        listbox row:selected {
            background-color: #4a9eff;
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
    GtkWidget* mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(mainBox, "main-container");
    gtk_window_set_child(GTK_WINDOW(m_window), mainBox);
    
    // Menu bar
    setupMenuBar();
    
    // Focus checkbox
    m_focusCheckbox = gtk_check_button_new_with_label("Set vibrance only when program is in focus");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(m_focusCheckbox), m_controller->getFocusMode());
    g_signal_connect(m_focusCheckbox, "toggled", G_CALLBACK(onFocusToggled), this);
    gtk_box_append(GTK_BOX(mainBox), m_focusCheckbox);
    
    // Display tabs
    setupDisplayTabs();
    gtk_box_append(GTK_BOX(mainBox), m_notebook);
    
    // Program section
    setupProgramSection();
}

void MainWindow::setupMenuBar() {
    GtkWidget* menuBar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    
    GtkWidget* fileMenu = gtk_menu_button_new();
    gtk_menu_button_set_label(GTK_MENU_BUTTON(fileMenu), "File");
    
    GtkWidget* helpMenu = gtk_menu_button_new();
    gtk_menu_button_set_label(GTK_MENU_BUTTON(helpMenu), "Help");
    
    gtk_box_append(GTK_BOX(menuBar), fileMenu);
    gtk_box_append(GTK_BOX(menuBar), helpMenu);
    
    // Add to window header
    GtkWidget* headerBar = gtk_header_bar_new();
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerBar), menuBar);
    gtk_window_set_titlebar(GTK_WINDOW(m_window), headerBar);
}

void MainWindow::setupDisplayTabs() {
    m_notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(m_notebook), GTK_POS_TOP);
    
    auto displays = m_controller->getDisplays();
    for (const auto& display : displays) {
        setupDisplayTab(display);
    }
}

void MainWindow::setupDisplayTab(const Display& display) {
    // Tab content
    GtkWidget* tabBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(tabBox, 16);
    gtk_widget_set_margin_end(tabBox, 16);
    gtk_widget_set_margin_top(tabBox, 16);
    gtk_widget_set_margin_bottom(tabBox, 16);
    
    // Vibrance container
    GtkWidget* vibranceContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(vibranceContainer, "vibrance-container");
    
    // Title
    std::string titleText = "Vibrance for " + display.id;
    GtkWidget* titleLabel = gtk_label_new(titleText.c_str());
    gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
    
    // Horizontal container for slider and spin button
    GtkWidget* controlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    
    // Vibrance slider
    GtkWidget* vibranceScale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -100.0, 100.0, 1.0);
    gtk_scale_set_draw_value(GTK_SCALE(vibranceScale), FALSE);
    gtk_range_set_value(GTK_RANGE(vibranceScale), display.currentVibrance);
    gtk_widget_set_hexpand(vibranceScale, TRUE);
    g_signal_connect(vibranceScale, "value-changed", G_CALLBACK(onVibranceChanged), this);
    
    // Vibrance spin button
    GtkWidget* vibranceSpin = gtk_spin_button_new_with_range(-100.0, 100.0, 1.0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(vibranceSpin), display.currentVibrance);
    g_signal_connect(vibranceSpin, "value-changed", G_CALLBACK(onVibranceSpinChanged), this);
    
    // Store references
    m_vibranceScales[display.id] = vibranceScale;
    m_vibranceSpins[display.id] = vibranceSpin;
    
    gtk_box_append(GTK_BOX(controlBox), vibranceScale);
    gtk_box_append(GTK_BOX(controlBox), vibranceSpin);
    
    gtk_box_append(GTK_BOX(vibranceContainer), titleLabel);
    gtk_box_append(GTK_BOX(vibranceContainer), controlBox);
    
    gtk_box_append(GTK_BOX(tabBox), vibranceContainer);
    
    // Tab label
    GtkWidget* tabLabel = gtk_label_new(display.id.c_str());
    
    // Add to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), tabBox, tabLabel);
}

void MainWindow::setupProgramSection() {
    GtkWidget* mainBox = gtk_widget_get_parent(m_notebook);
    
    // Button container
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(buttonBox, 8);
    gtk_widget_set_margin_end(buttonBox, 8);
    
    m_addProgramButton = gtk_button_new_with_label("Add program");
    g_signal_connect(m_addProgramButton, "clicked", G_CALLBACK(onAddProgram), this);
    
    m_removeProgramButton = gtk_button_new_with_label("Remove program");
    gtk_widget_add_css_class(m_removeProgramButton, "remove-button");
    g_signal_connect(m_removeProgramButton, "clicked", G_CALLBACK(onRemoveProgram), this);
    
    gtk_box_append(GTK_BOX(buttonBox), m_addProgramButton);
    gtk_box_append(GTK_BOX(buttonBox), m_removeProgramButton);
    
    // Program list
    GtkWidget* scrolled = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrolled, -1, 120);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    m_programListBox = gtk_list_box_new();
    g_signal_connect(m_programListBox, "row-activated", G_CALLBACK(onProgramDoubleClick), this);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), m_programListBox);
    
    gtk_widget_set_margin_start(scrolled, 8);
    gtk_widget_set_margin_end(scrolled, 8);
    gtk_widget_set_margin_bottom(scrolled, 8);
    
    gtk_box_append(GTK_BOX(mainBox), buttonBox);
    gtk_box_append(GTK_BOX(mainBox), scrolled);
}

void MainWindow::updateProgramList() {
    // Clear existing items
    GtkWidget* child = gtk_widget_get_first_child(m_programListBox);
    while (child) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(GTK_LIST_BOX(m_programListBox), child);
        child = next;
    }
    
    // Add profiles
    auto profiles = m_controller->getProfiles();
    for (const auto& profile : profiles) {
        GtkWidget* label = gtk_label_new(profile.name.c_str());
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_list_box_append(GTK_LIST_BOX(m_programListBox), label);
    }
}

// Event handlers
void MainWindow::onVibranceChanged(GtkRange* range, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    double value = gtk_range_get_value(range);
    int vibrance = static_cast<int>(value);
    
    // Find which display this belongs to
    for (const auto& pair : window->m_vibranceScales) {
        if (pair.second == GTK_WIDGET(range)) {
            // Update spin button
            auto spinIt = window->m_vibranceSpins.find(pair.first);
            if (spinIt != window->m_vibranceSpins.end()) {
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinIt->second), vibrance);
            }
            
            // Apply vibrance
            window->m_controller->setVibrance(pair.first, vibrance);
            break;
        }
    }
}

void MainWindow::onVibranceSpinChanged(GtkSpinButton* spin, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    double value = gtk_spin_button_get_value(spin);
    int vibrance = static_cast<int>(value);
    
    // Find which display this belongs to
    for (const auto& pair : window->m_vibranceSpins) {
        if (pair.second == GTK_WIDGET(spin)) {
            // Update scale
            auto scaleIt = window->m_vibranceScales.find(pair.first);
            if (scaleIt != window->m_vibranceScales.end()) {
                gtk_range_set_value(GTK_RANGE(scaleIt->second), vibrance);
            }
            
            // Apply vibrance
            window->m_controller->setVibrance(pair.first, vibrance);
            break;
        }
    }
}

void MainWindow::onFocusToggled(GtkCheckButton* button, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    bool enabled = gtk_check_button_get_active(button);
    window->m_controller->setFocusMode(enabled);
}

void MainWindow::onAddProgram(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    GtkFileDialog* dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Select Program");
    
    gtk_file_dialog_open(dialog, GTK_WINDOW(window->m_window), nullptr,
        [](GObject* source, GAsyncResult* result, gpointer user_data) {
            auto* window = static_cast<MainWindow*>(user_data);
            GtkFileDialog* dialog = GTK_FILE_DIALOG(source);
            
            GFile* file = gtk_file_dialog_open_finish(dialog, result, nullptr);
            if (file) {
                char* path = g_file_get_path(file);
                if (path) {
                    window->showProgramDialog(std::string(path));
                    g_free(path);
                }
                g_object_unref(file);
            }
            g_object_unref(dialog);
        }, window);
}

void MainWindow::onRemoveProgram(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    GtkListBoxRow* selected = gtk_list_box_get_selected_row(GTK_LIST_BOX(window->m_programListBox));
    if (selected) {
        GtkWidget* label = gtk_list_box_row_get_child(selected);
        if (label) {
            const char* text = gtk_label_get_text(GTK_LABEL(label));
            window->m_controller->removeProfile(text);
            window->updateProgramList();
        }
    }
}

void MainWindow::onProgramDoubleClick(GtkListBox* listbox __attribute__((unused)), GtkListBoxRow* row, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    GtkWidget* label = gtk_list_box_row_get_child(row);
    if (label) {
        const char* text = gtk_label_get_text(GTK_LABEL(label));
        
        // Find profile and edit it
        auto profiles = window->m_controller->getProfiles();
        for (const auto& profile : profiles) {
            if (profile.name == text) {
                window->showProgramDialog(profile.path);
                break;
            }
        }
    }
}

void MainWindow::showProgramDialog(const std::string& programPath) {
    // This will show the program configuration dialog matching the second image
    // Implementation would create the dialog shown in entryeditor.png
    std::cout << "📝 Opening program dialog for: " << programPath << std::endl;
    
    // For now, create a simple profile
    if (!programPath.empty()) {
        ProgramProfile profile;
        profile.name = std::filesystem::path(programPath).filename().string();
        profile.path = programPath;
        profile.pathMatching = true;
        profile.enabled = true;
        
        // Set default vibrance for all displays
        auto displays = m_controller->getDisplays();
        for (const auto& display : displays) {
            profile.displayVibrance[display.id] = 50; // Default to +50 vibrance
        }
        
        m_controller->saveProfile(profile);
        updateProgramList();
    }
}

void MainWindow::show() {
    gtk_window_present(GTK_WINDOW(m_window));
}
