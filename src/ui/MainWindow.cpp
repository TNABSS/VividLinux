#include "MainWindow.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

MainWindow::MainWindow(GtkApplication* app, VividManager* manager) 
    : m_manager(manager) {
    
    m_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(m_window), "Vivid - Digital Vibrance Control");
    gtk_window_set_default_size(GTK_WINDOW(m_window), 600, 500);
    gtk_window_set_resizable(GTK_WINDOW(m_window), TRUE);
    
    applyModernTheme();
    setupUI();
    updateVibranceControls();
    updateProgramList();
}

MainWindow::~MainWindow() = default;

void MainWindow::applyModernTheme() {
    GtkCssProvider* provider = gtk_css_provider_new();
    
    const char* css = R"(
        window {
            background: linear-gradient(135deg, #1a1a1a 0%, #2d2d2d 100%);
            color: #ffffff;
            font-family: 'Segoe UI', system-ui, sans-serif;
        }
        
        .main-container {
            background: rgba(45, 45, 45, 0.95);
            border-radius: 12px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
        }
        
        .card {
            background: rgba(60, 60, 60, 0.8);
            border-radius: 8px;
            border: 1px solid rgba(255, 255, 255, 0.1);
            box-shadow: 0 4px 16px rgba(0, 0, 0, 0.2);
            margin: 8px;
            padding: 16px;
        }
        
        .display-tab {
            background: linear-gradient(135deg, #404040 0%, #505050 100%);
            border: 2px solid rgba(255, 255, 255, 0.1);
            border-radius: 8px;
            padding: 12px 20px;
            margin: 4px;
            font-weight: 600;
            transition: all 0.2s ease;
        }
        
        .display-tab:checked {
            background: linear-gradient(135deg, #0078d4 0%, #106ebe 100%);
            border-color: #ffffff;
            box-shadow: 0 4px 12px rgba(0, 120, 212, 0.4);
        }
        
        .vibrance-scale {
            min-height: 40px;
            margin: 8px 0;
        }
        
        .vibrance-scale trough {
            background: linear-gradient(90deg, #ff4444 0%, #ffff44 50%, #44ff44 100%);
            border-radius: 20px;
            min-height: 8px;
            border: 2px solid rgba(255, 255, 255, 0.2);
        }
        
        .vibrance-scale slider {
            background: radial-gradient(circle, #ffffff 0%, #e0e0e0 100%);
            border: 3px solid #0078d4;
            border-radius: 50%;
            min-width: 24px;
            min-height: 24px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
        }
        
        .vibrance-scale slider:hover {
            background: radial-gradient(circle, #ffffff 0%, #f0f0f0 100%);
            transform: scale(1.1);
        }
        
        .program-list {
            background: rgba(30, 30, 30, 0.8);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 8px;
            min-height: 120px;
        }
        
        .program-list row {
            background: transparent;
            border-radius: 6px;
            margin: 2px;
            padding: 8px 12px;
        }
        
        .program-list row:hover {
            background: rgba(255, 255, 255, 0.1);
        }
        
        .program-list row:selected {
            background: rgba(0, 120, 212, 0.3);
        }
        
        .modern-button {
            background: linear-gradient(135deg, #0078d4 0%, #106ebe 100%);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-weight: 600;
            box-shadow: 0 4px 12px rgba(0, 120, 212, 0.3);
            transition: all 0.2s ease;
        }
        
        .modern-button:hover {
            background: linear-gradient(135deg, #106ebe 0%, #005a9e 100%);
            transform: translateY(-2px);
            box-shadow: 0 6px 16px rgba(0, 120, 212, 0.4);
        }
        
        .danger-button {
            background: linear-gradient(135deg, #d13438 0%, #b71c1c 100%);
            box-shadow: 0 4px 12px rgba(209, 52, 56, 0.3);
        }
        
        .danger-button:hover {
            background: linear-gradient(135deg, #b71c1c 0%, #8e0000 100%);
            box-shadow: 0 6px 16px rgba(209, 52, 56, 0.4);
        }
        
        .status-bar {
            background: rgba(40, 40, 40, 0.9);
            border-top: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 0 0 12px 12px;
            padding: 12px 16px;
            font-family: 'JetBrains Mono', 'Consolas', monospace;
            font-size: 0.9em;
        }
        
        .vibrance-value {
            background: rgba(0, 120, 212, 0.2);
            border: 1px solid #0078d4;
            border-radius: 6px;
            padding: 8px 12px;
            font-weight: 600;
            font-family: 'JetBrains Mono', monospace;
            min-width: 80px;
        }
        
        .safety-warning {
            background: rgba(255, 193, 7, 0.2);
            border: 1px solid #ffc107;
            border-radius: 6px;
            padding: 8px 12px;
            color: #ffc107;
            font-weight: 500;
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
    // Main container with modern styling
    GtkWidget* mainContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(mainContainer, "main-container");
    gtk_widget_set_margin_start(mainContainer, 16);
    gtk_widget_set_margin_end(mainContainer, 16);
    gtk_widget_set_margin_top(mainContainer, 16);
    gtk_widget_set_margin_bottom(mainContainer, 16);
    gtk_window_set_child(GTK_WINDOW(m_window), mainContainer);
    
    // Modern header bar
    setupModernHeader();
    
    // Content area
    GtkWidget* contentBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(contentBox, 20);
    gtk_widget_set_margin_end(contentBox, 20);
    gtk_widget_set_margin_top(contentBox, 20);
    gtk_widget_set_margin_bottom(contentBox, 20);
    gtk_box_append(GTK_BOX(mainContainer), contentBox);
    
    // Safety warning
    setupSafetyWarning(contentBox);
    
    // Focus control card
    setupFocusCard(contentBox);
    
    // Display control tabs
    setupDisplayTabs();
    gtk_box_append(GTK_BOX(contentBox), m_notebook);
    
    // Program management card
    setupProgramCard(contentBox);
    
    // Status bar
    setupStatusBar(mainContainer);
}

void MainWindow::setupModernHeader() {
    GtkWidget* headerBar = gtk_header_bar_new();
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(headerBar), 
        gtk_label_new("🎮 Vivid - Digital Vibrance Control"));
    gtk_window_set_titlebar(GTK_WINDOW(m_window), headerBar);
    
    // Add safety reset button to header
    GtkWidget* resetAllButton = gtk_button_new_with_label("🔄 Reset All");
    gtk_widget_add_css_class(resetAllButton, "danger-button");
    g_signal_connect(resetAllButton, "clicked", G_CALLBACK(onResetAllClicked), this);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerBar), resetAllButton);
    
    // Add menu button
    GtkWidget* menuButton = gtk_menu_button_new();
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerBar), menuButton);
    
    GMenu* menu = g_menu_new();
    g_menu_append(menu, "About Vivid", "app.about");
    g_menu_append(menu, "Safety Guide", "app.safety");
    g_menu_append(menu, "Quit", "app.quit");
    
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menuButton), G_MENU_MODEL(menu));
    g_object_unref(menu);
}

void MainWindow::setupSafetyWarning(GtkWidget* parent) {
    GtkWidget* warningBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_add_css_class(warningBox, "safety-warning");
    
    GtkWidget* warningIcon = gtk_label_new("⚠️");
    GtkWidget* warningText = gtk_label_new(
        "Safe Mode Active: Changes are limited to prevent system damage. "
        "All settings can be instantly reset."
    );
    gtk_label_set_wrap(GTK_LABEL(warningText), TRUE);
    
    gtk_box_append(GTK_BOX(warningBox), warningIcon);
    gtk_box_append(GTK_BOX(warningBox), warningText);
    gtk_box_append(GTK_BOX(parent), warningBox);
}

void MainWindow::setupFocusCard(GtkWidget* parent) {
    GtkWidget* focusCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_add_css_class(focusCard, "card");
    
    GtkWidget* focusTitle = gtk_label_new("🎯 Application Focus Control");
    gtk_widget_set_halign(focusTitle, GTK_ALIGN_START);
    gtk_label_set_markup(GTK_LABEL(focusTitle), "<b>🎯 Application Focus Control</b>");
    
    m_focusCheckbox = gtk_check_button_new_with_label(
        "Only apply vibrance when specific programs are active"
    );
    g_signal_connect(m_focusCheckbox, "toggled", G_CALLBACK(onFocusCheckboxToggled), this);
    
    gtk_box_append(GTK_BOX(focusCard), focusTitle);
    gtk_box_append(GTK_BOX(focusCard), m_focusCheckbox);
    gtk_box_append(GTK_BOX(parent), focusCard);
}

void MainWindow::setupDisplayTabs() {
    m_notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(m_notebook), GTK_POS_TOP);
    gtk_widget_add_css_class(m_notebook, "card");
    
    auto displays = m_manager->getDisplays();
    for (const auto& display : displays) {
        setupDisplayTab(display);
    }
    
    g_signal_connect(m_notebook, "switch-page", G_CALLBACK(onDisplayTabChanged), this);
}

void MainWindow::setupDisplayTab(const VividDisplay& display) {
    // Tab content
    GtkWidget* tabContent = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(tabContent, 24);
    gtk_widget_set_margin_end(tabContent, 24);
    gtk_widget_set_margin_top(tabContent, 24);
    gtk_widget_set_margin_bottom(tabContent, 24);
    
    // Display info
    GtkWidget* displayInfo = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    
    std::string titleText = "🖥️ " + display.name;
    GtkWidget* displayTitle = gtk_label_new(titleText.c_str());
    gtk_label_set_markup(GTK_LABEL(displayTitle), 
        ("<span size='large' weight='bold'>" + titleText + "</span>").c_str());
    gtk_widget_set_halign(displayTitle, GTK_ALIGN_START);
    
    std::string infoText = "Connector: " + display.connector + " | Status: Connected";
    GtkWidget* displayDetails = gtk_label_new(infoText.c_str());
    gtk_widget_set_halign(displayDetails, GTK_ALIGN_START);
    
    gtk_box_append(GTK_BOX(displayInfo), displayTitle);
    gtk_box_append(GTK_BOX(displayInfo), displayDetails);
    gtk_box_append(GTK_BOX(tabContent), displayInfo);
    
    // Vibrance control section
    GtkWidget* vibranceSection = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    
    GtkWidget* vibranceLabel = gtk_label_new("🎨 Digital Vibrance Control");
    gtk_label_set_markup(GTK_LABEL(vibranceLabel), "<b>🎨 Digital Vibrance Control</b>");
    gtk_widget_set_halign(vibranceLabel, GTK_ALIGN_START);
    
    // Control container
    GtkWidget* controlContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    
    // Current value display
    GtkWidget* valueDisplay = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(valueDisplay, GTK_ALIGN_CENTER);
    
    GtkWidget* currentLabel = gtk_label_new("Current Value:");
    GtkWidget* currentValue = gtk_label_new("0");
    gtk_widget_add_css_class(currentValue, "vibrance-value");
    m_vibranceLabels[display.id] = currentValue;
    
    gtk_box_append(GTK_BOX(valueDisplay), currentLabel);
    gtk_box_append(GTK_BOX(valueDisplay), currentValue);
    
    // Vibrance scale with better layout
    GtkWidget* scaleContainer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    
    GtkWidget* minLabel = gtk_label_new("-100\n(Gray)");
    gtk_label_set_justify(GTK_LABEL(minLabel), GTK_JUSTIFY_CENTER);
    
    GtkWidget* vibranceScale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -100.0, 100.0, 1.0);
    gtk_widget_add_css_class(vibranceScale, "vibrance-scale");
    gtk_scale_set_draw_value(GTK_SCALE(vibranceScale), FALSE);
    gtk_range_set_value(GTK_RANGE(vibranceScale), display.currentVibrance);
    gtk_widget_set_hexpand(vibranceScale, TRUE);
    
    // Add scale marks
    gtk_scale_add_mark(GTK_SCALE(vibranceScale), -100.0, GTK_POS_BOTTOM, nullptr);
    gtk_scale_add_mark(GTK_SCALE(vibranceScale), -50.0, GTK_POS_BOTTOM, nullptr);
    gtk_scale_add_mark(GTK_SCALE(vibranceScale), 0.0, GTK_POS_BOTTOM, nullptr);
    gtk_scale_add_mark(GTK_SCALE(vibranceScale), 50.0, GTK_POS_BOTTOM, nullptr);
    gtk_scale_add_mark(GTK_SCALE(vibranceScale), 100.0, GTK_POS_BOTTOM, nullptr);
    
    GtkWidget* maxLabel = gtk_label_new("+100\n(Vivid)");
    gtk_label_set_justify(GTK_LABEL(maxLabel), GTK_JUSTIFY_CENTER);
    
    g_signal_connect(vibranceScale, "value-changed", G_CALLBACK(onVibranceChanged), this);
    
    gtk_box_append(GTK_BOX(scaleContainer), minLabel);
    gtk_box_append(GTK_BOX(scaleContainer), vibranceScale);
    gtk_box_append(GTK_BOX(scaleContainer), maxLabel);
    
    // Quick preset buttons
    GtkWidget* presetBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(presetBox, GTK_ALIGN_CENTER);
    
    const std::vector<std::pair<std::string, int>> presets = {
        {"📷 Photo", -30}, {"📺 Normal", 0}, {"🎮 Gaming", 50}, {"🌈 Max", 100}
    };
    
    for (const auto& preset : presets) {
        GtkWidget* presetBtn = gtk_button_new_with_label(preset.first.c_str());
        gtk_widget_add_css_class(presetBtn, "modern-button");
        g_object_set_data(G_OBJECT(presetBtn), "display-id", g_strdup(display.id.c_str()));
        g_object_set_data(G_OBJECT(presetBtn), "preset-value", GINT_TO_POINTER(preset.second));
        g_signal_connect(presetBtn, "clicked", G_CALLBACK(onPresetClicked), this);
        gtk_box_append(GTK_BOX(presetBox), presetBtn);
    }
    
    // Store references
    m_vibranceScales[display.id] = vibranceScale;
    
    // Assemble vibrance section
    gtk_box_append(GTK_BOX(vibranceSection), vibranceLabel);
    gtk_box_append(GTK_BOX(vibranceSection), valueDisplay);
    gtk_box_append(GTK_BOX(vibranceSection), scaleContainer);
    gtk_box_append(GTK_BOX(vibranceSection), presetBox);
    
    gtk_box_append(GTK_BOX(tabContent), vibranceSection);
    
    // Create tab label
    GtkWidget* tabLabel = gtk_label_new(("🖥️ " + display.id).c_str());
    gtk_widget_add_css_class(tabLabel, "display-tab");
    
    // Add tab to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), tabContent, tabLabel);
    m_displayTabs[display.id] = tabContent;
}

void MainWindow::setupProgramCard(GtkWidget* parent) {
    GtkWidget* programCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(programCard, "card");
    
    // Title
    GtkWidget* programTitle = gtk_label_new("🎯 Per-Application Profiles");
    gtk_label_set_markup(GTK_LABEL(programTitle), "<b>🎯 Per-Application Profiles</b>");
    gtk_widget_set_halign(programTitle, GTK_ALIGN_START);
    
    // Button container
    GtkWidget* buttonContainer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    
    m_addProgramButton = gtk_button_new_with_label("📁 Browse & Add Program");
    gtk_widget_add_css_class(m_addProgramButton, "modern-button");
    g_signal_connect(m_addProgramButton, "clicked", G_CALLBACK(onAddProgramClicked), this);
    
    m_removeProgramButton = gtk_button_new_with_label("🗑️ Remove Selected");
    gtk_widget_add_css_class(m_removeProgramButton, "danger-button");
    g_signal_connect(m_removeProgramButton, "clicked", G_CALLBACK(onRemoveProgramClicked), this);
    
    gtk_box_append(GTK_BOX(buttonContainer), m_addProgramButton);
    gtk_box_append(GTK_BOX(buttonContainer), m_removeProgramButton);
    
    // Program list
    setupProgramList();
    
    // Assemble program card
    gtk_box_append(GTK_BOX(programCard), programTitle);
    gtk_box_append(GTK_BOX(programCard), buttonContainer);
    gtk_box_append(GTK_BOX(programCard), m_programList);
    
    gtk_box_append(GTK_BOX(parent), programCard);
}

void MainWindow::setupProgramList() {
    GtkWidget* scrolled = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrolled, -1, 150);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), 
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    GtkWidget* listBox = gtk_list_box_new();
    gtk_widget_add_css_class(listBox, "program-list");
    
    m_programListBox = listBox;
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), listBox);
    m_programList = scrolled;
}

void MainWindow::setupStatusBar(GtkWidget* parent) {
    m_statusLabel = gtk_label_new("");
    gtk_widget_add_css_class(m_statusLabel, "status-bar");
    gtk_widget_set_halign(m_statusLabel, GTK_ALIGN_START);
    
    std::string status = "🔧 Method: " + m_manager->getMethodName() + 
                        " | 🛡️ Safe Mode: Active | 🎮 Ready for use";
    gtk_label_set_text(GTK_LABEL(m_statusLabel), status.c_str());
    
    gtk_box_append(GTK_BOX(parent), m_statusLabel);
}

// Event handlers
void MainWindow::onVibranceChanged(GtkRange* range, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    for (const auto& pair : window->m_vibranceScales) {
        if (pair.second == GTK_WIDGET(range)) {
            double value = gtk_range_get_value(range);
            
            // Safety check: limit extreme values
            if (value < -100) value = -100;
            if (value > 100) value = 100;
            
            // Update display
            auto labelIt = window->m_vibranceLabels.find(pair.first);
            if (labelIt != window->m_vibranceLabels.end()) {
                std::string valueText = std::to_string((int)value);
                if (value > 0) valueText = "+" + valueText;
                gtk_label_set_text(GTK_LABEL(labelIt->second), valueText.c_str());
            }
            
            // Apply vibrance with safety checks
            window->m_manager->setVibranceSafe(pair.first, static_cast<float>(value));
            break;
        }
    }
}

void MainWindow::onPresetClicked(GtkButton* button, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    const char* displayId = static_cast<const char*>(g_object_get_data(G_OBJECT(button), "display-id"));
    int presetValue = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "preset-value"));
    
    if (displayId) {
        // Update scale
        auto scaleIt = window->m_vibranceScales.find(displayId);
        if (scaleIt != window->m_vibranceScales.end()) {
            gtk_range_set_value(GTK_RANGE(scaleIt->second), presetValue);
        }
        
        // Apply vibrance
        window->m_manager->setVibranceSafe(displayId, static_cast<float>(presetValue));
    }
}

void MainWindow::onResetAllClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    // Show confirmation dialog
    GtkWidget* dialog = gtk_message_dialog_new(
        GTK_WINDOW(window->m_window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "Reset all displays to normal vibrance?"
    );
    
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
        "This will reset all displays to 0 vibrance (normal colors).");
    
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_window_destroy(GTK_WINDOW(dialog));
    
    if (response == GTK_RESPONSE_YES) {
        // Reset all displays
        for (const auto& pair : window->m_vibranceScales) {
            gtk_range_set_value(GTK_RANGE(pair.second), 0.0);
            window->m_manager->resetVibrance(pair.first);
        }
        
        std::cout << "🔄 All displays reset to normal vibrance" << std::endl;
    }
}

void MainWindow::onAddProgramClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    window->showFileBrowserDialog();
}

void MainWindow::showFileBrowserDialog() {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Select Program to Add",
        GTK_WINDOW(m_window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Add Program", GTK_RESPONSE_ACCEPT,
        nullptr
    );
    
    // Set up file filters
    GtkFileFilter* executableFilter = gtk_file_filter_new();
    gtk_file_filter_set_name(executableFilter, "Executable Files");
    gtk_file_filter_add_mime_type(executableFilter, "application/x-executable");
    gtk_file_filter_add_pattern(executableFilter, "*.exe");
    gtk_file_filter_add_pattern(executableFilter, "*.AppImage");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), executableFilter);
    
    GtkFileFilter* allFilter = gtk_file_filter_new();
    gtk_file_filter_set_name(allFilter, "All Files");
    gtk_file_filter_add_pattern(allFilter, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), allFilter);
    
    // Set common directories
    gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog), "/usr/bin", nullptr);
    gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog), "/usr/local/bin", nullptr);
    gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(dialog), g_get_home_dir(), nullptr);
    
    // Show dialog
    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
        char* filename = gtk_file_chooser_get_filename(chooser);
        
        if (filename) {
            showProgramConfigDialog(filename);
            g_free(filename);
        }
    }
    
    gtk_window_destroy(GTK_WINDOW(dialog));
}

void MainWindow::showProgramConfigDialog(const std::string& programPath) {
    GtkWidget* dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Configure Program Profile");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(m_window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    
    // Main container
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(vbox, 24);
    gtk_widget_set_margin_end(vbox, 24);
    gtk_widget_set_margin_top(vbox, 24);
    gtk_widget_set_margin_bottom(vbox, 24);
    gtk_window_set_child(GTK_WINDOW(dialog), vbox);
    
    // Program info
    std::string programName = std::filesystem::path(programPath).filename().string();
    GtkWidget* titleLabel = gtk_label_new(("Configure: " + programName).c_str());
    gtk_label_set_markup(GTK_LABEL(titleLabel), 
        ("<span size='large' weight='bold'>Configure: " + programName + "</span>").c_str());
    
    GtkWidget* pathLabel = gtk_label_new(("Path: " + programPath).c_str());
    gtk_label_set_wrap(GTK_LABEL(pathLabel), TRUE);
    
    // Profile name entry
    GtkWidget* nameBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget* nameLabel = gtk_label_new("Profile Name:");
    GtkWidget* nameEntry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(nameEntry), programName.c_str());
    gtk_widget_set_hexpand(nameEntry, TRUE);
    
    gtk_box_append(GTK_BOX(nameBox), nameLabel);
    gtk_box_append(GTK_BOX(nameBox), nameEntry);
    
    // Vibrance settings for each display
    GtkWidget* vibranceFrame = gtk_frame_new("Vibrance Settings");
    GtkWidget* vibranceBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(vibranceBox, 12);
    gtk_widget_set_margin_end(vibranceBox, 12);
    gtk_widget_set_margin_top(vibranceBox, 12);
    gtk_widget_set_margin_bottom(vibranceBox, 12);
    
    std::map<std::string, GtkWidget*> displayScales;
    auto displays = m_manager->getDisplays();
    
    for (const auto& display : displays) {
        GtkWidget* displayBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        
        GtkWidget* displayLabel = gtk_label_new((display.name + ":").c_str());
        gtk_widget_set_size_request(displayLabel, 120, -1);
        
        GtkWidget* scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -100.0, 100.0, 1.0);
        gtk_range_set_value(GTK_RANGE(scale), 0.0);
        gtk_widget_set_hexpand(scale, TRUE);
        
        displayScales[display.id] = scale;
        
        gtk_box_append(GTK_BOX(displayBox), displayLabel);
        gtk_box_append(GTK_BOX(displayBox), scale);
        gtk_box_append(GTK_BOX(vibranceBox), displayBox);
    }
    
    gtk_frame_set_child(GTK_FRAME(vibranceFrame), vibranceBox);
    
    // Buttons
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(buttonBox, GTK_ALIGN_END);
    
    GtkWidget* cancelButton = gtk_button_new_with_label("Cancel");
    GtkWidget* saveButton = gtk_button_new_with_label("Save Profile");
    gtk_widget_add_css_class(saveButton, "modern-button");
    
    gtk_box_append(GTK_BOX(buttonBox), cancelButton);
    gtk_box_append(GTK_BOX(buttonBox), saveButton);
    
    // Assemble dialog
    gtk_box_append(GTK_BOX(vbox), titleLabel);
    gtk_box_append(GTK_BOX(vbox), pathLabel);
    gtk_box_append(GTK_BOX(vbox), nameBox);
    gtk_box_append(GTK_BOX(vbox), vibranceFrame);
    gtk_box_append(GTK_BOX(vbox), buttonBox);
    
    // Connect signals
    g_signal_connect_swapped(cancelButton, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    
    // Save button callback with data
    g_object_set_data_full(G_OBJECT(saveButton), "program-path", g_strdup(programPath.c_str()), g_free);
    g_object_set_data(G_OBJECT(saveButton), "name-entry", nameEntry);
    g_object_set_data(G_OBJECT(saveButton), "dialog", dialog);
    g_object_set_data(G_OBJECT(saveButton), "main-window", this);
    
    // Store display scales
    for (const auto& pair : displayScales) {
        std::string key = "scale-" + pair.first;
        g_object_set_data(G_OBJECT(saveButton), key.c_str(), pair.second);
    }
    
    g_signal_connect(saveButton, "clicked", G_CALLBACK(onSaveProgramProfile), nullptr);
    
    gtk_window_present(GTK_WINDOW(dialog));
}

void MainWindow::onSaveProgramProfile(GtkButton* button, gpointer user_data __attribute__((unused))) {
    const char* programPath = static_cast<const char*>(g_object_get_data(G_OBJECT(button), "program-path"));
    GtkEntry* nameEntry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "name-entry"));
    GtkWidget* dialog = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "dialog"));
    MainWindow* window = static_cast<MainWindow*>(g_object_get_data(G_OBJECT(button), "main-window"));
    
    if (programPath && nameEntry && window) {
        AppProfile profile;
        profile.name = gtk_entry_get_text(nameEntry);
        profile.executable = programPath;
        profile.enabled = true;
        
        // Get vibrance settings for each display
        auto displays = window->m_manager->getDisplays();
        for (const auto& display : displays) {
            std::string key = "scale-" + display.id;
            GtkRange* scale = GTK_RANGE(g_object_get_data(G_OBJECT(button), key.c_str()));
            if (scale) {
                double value = gtk_range_get_value(scale);
                profile.displayVibrance[display.id] = static_cast<float>(value);
            }
        }
        
        // Save profile
        window->m_manager->saveProfile(profile);
        window->updateProgramList();
        
        std::cout << "✅ Saved profile: " << profile.name << std::endl;
    }
    
    if (dialog) {
        gtk_window_destroy(GTK_WINDOW(dialog));
    }
}

// Other event handlers...
void MainWindow::onRemoveProgramClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    if (window->m_programListBox) {
        GtkListBoxRow* selected = gtk_list_box_get_selected_row(GTK_LIST_BOX(window->m_programListBox));
        if (selected) {
            gtk_list_box_remove(GTK_LIST_BOX(window->m_programListBox), GTK_WIDGET(selected));
        }
    }
}

void MainWindow::onDisplayTabChanged(GtkNotebook* notebook __attribute__((unused)), GtkWidget* page __attribute__((unused)), guint page_num, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
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

void MainWindow::updateVibranceControls() {
    auto displays = m_manager->getDisplays();
    for (const auto& display : displays) {
        auto scaleIt = m_vibranceScales.find(display.id);
        if (scaleIt != m_vibranceScales.end()) {
            gtk_range_set_value(GTK_RANGE(scaleIt->second), display.currentVibrance);
        }
        
        auto labelIt = m_vibranceLabels.find(display.id);
        if (labelIt != m_vibranceLabels.end()) {
            std::string valueText = std::to_string((int)display.currentVibrance);
            if (display.currentVibrance > 0) valueText = "+" + valueText;
            gtk_label_set_text(GTK_LABEL(labelIt->second), valueText.c_str());
        }
    }
}

void MainWindow::updateProgramList() {
    if (!m_programListBox) return;
    
    // Clear existing items
    GtkWidget* child = gtk_widget_get_first_child(m_programListBox);
    while (child) {
        GtkWidget* next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(GTK_LIST_BOX(m_programListBox), child);
        child = next;
    }
    
    // Add profiles
    auto profiles = m_manager->getProfiles();
    for (const auto& profile : profiles) {
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
        
        GtkWidget* icon = gtk_label_new("🎮");
        GtkWidget* nameLabel = gtk_label_new(profile.name.c_str());
        gtk_widget_set_hexpand(nameLabel, TRUE);
        gtk_widget_set_halign(nameLabel, GTK_ALIGN_START);
        
        GtkWidget* pathLabel = gtk_label_new(std::filesystem::path(profile.executable).filename().string().c_str());
        gtk_label_set_markup(GTK_LABEL(pathLabel), 
            ("<span size='small' alpha='70%'>" + 
             std::filesystem::path(profile.executable).filename().string() + "</span>").c_str());
        
        gtk_box_append(GTK_BOX(row), icon);
        gtk_box_append(GTK_BOX(row), nameLabel);
        gtk_box_append(GTK_BOX(row), pathLabel);
        
        gtk_list_box_append(GTK_LIST_BOX(m_programListBox), row);
    }
    
    if (profiles.empty()) {
        GtkWidget* emptyRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget* emptyIcon = gtk_label_new("📁");
        GtkWidget* emptyLabel = gtk_label_new("No programs added yet - click 'Browse & Add Program' to get started");
        gtk_label_set_markup(GTK_LABEL(emptyLabel), 
            "<span alpha='70%'>No programs added yet - click 'Browse & Add Program' to get started</span>");
        
        gtk_box_append(GTK_BOX(emptyRow), emptyIcon);
        gtk_box_append(GTK_BOX(emptyRow), emptyLabel);
        gtk_list_box_append(GTK_LIST_BOX(m_programListBox), emptyRow);
    }
}

void MainWindow::show() {
    gtk_window_present(GTK_WINDOW(m_window));
}
