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
    
    // Autostart management card
    setupAutostartCard(contentBox);
    
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

void MainWindow::setupAutostartCard(GtkWidget* parent) {
    GtkWidget* autostartCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_add_css_class(autostartCard, "card");
    
    // Title
    GtkWidget* autostartTitle = gtk_label_new("🚀 Autostart Configuration");
    gtk_label_set_markup(GTK_LABEL(autostartTitle), "<b>🚀 Autostart Configuration</b>");
    gtk_widget_set_halign(autostartTitle, GTK_ALIGN_START);
    
    // Status display
    m_autostartStatusLabel = gtk_label_new("");
    gtk_widget_set_halign(m_autostartStatusLabel, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(m_autostartStatusLabel), TRUE);
    updateAutostartStatus();
    
    // Autostart checkbox
    m_autostartCheckbox = gtk_check_button_new_with_label(
        "Start Vivid automatically when I log in"
    );
    gtk_check_button_set_active(GTK_CHECK_BUTTON(m_autostartCheckbox), 
                               m_manager->isAutostartEnabled());
    g_signal_connect(m_autostartCheckbox, "toggled", G_CALLBACK(onAutostartToggled), this);
    
    // Options section
    GtkWidget* optionsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(optionsBox, 20);
    
    GtkWidget* minimizeCheck = gtk_check_button_new_with_label(
        "Start minimized to system tray"
    );
    gtk_widget_set_sensitive(minimizeCheck, FALSE); // TODO: Implement tray
    
    GtkWidget* profilesCheck = gtk_check_button_new_with_label(
        "Apply saved profiles on startup"
    );
    gtk_check_button_set_active(GTK_CHECK_BUTTON(profilesCheck), TRUE);
    
    GtkWidget* delayBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget* delayLabel = gtk_label_new("Startup delay:");
    GtkWidget* delaySpin = gtk_spin_button_new_with_range(0, 30, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(delaySpin), 3);
    GtkWidget* delayUnit = gtk_label_new("seconds");
    
    gtk_box_append(GTK_BOX(delayBox), delayLabel);
    gtk_box_append(GTK_BOX(delayBox), delaySpin);
    gtk_box_append(GTK_BOX(delayBox), delayUnit);
    
    gtk_box_append(GTK_BOX(optionsBox), minimizeCheck);
    gtk_box_append(GTK_BOX(optionsBox), profilesCheck);
    gtk_box_append(GTK_BOX(optionsBox), delayBox);
    
    // Debug button
    GtkWidget* debugButton = gtk_button_new_with_label("🔧 Debug Autostart");
    gtk_widget_add_css_class(debugButton, "modern-button");
    g_signal_connect(debugButton, "clicked", G_CALLBACK(onAutostartDebugClicked), this);
    
    // Help text
    GtkWidget* helpText = gtk_label_new(
        "Autostart allows Vivid to automatically apply your vibrance settings when you log in. "
        "This is useful for maintaining consistent colors across reboots."
    );
    gtk_label_set_wrap(GTK_LABEL(helpText), TRUE);
    gtk_label_set_markup(GTK_LABEL(helpText), 
        "<span size='small' alpha='80%'>Autostart allows Vivid to automatically apply your vibrance settings when you log in. "
        "This is useful for maintaining consistent colors across reboots.</span>");
    
    // Assemble card
    gtk_box_append(GTK_BOX(autostartCard), autostartTitle);
    gtk_box_append(GTK_BOX(autostartCard), m_autostartStatusLabel);
    gtk_box_append(GTK_BOX(autostartCard), m_autostartCheckbox);
    gtk_box_append(GTK_BOX(autostartCard), optionsBox);
    gtk_box_append(GTK_BOX(autostartCard), debugButton);
    gtk_box_append(GTK_BOX(autostartCard), helpText);
    
    gtk_box_append(GTK_BOX(parent), autostartCard);
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
    
    // Use modern GTK4 dialog approach
    GtkAlertDialog* dialog = gtk_alert_dialog_new("Reset all displays to normal vibrance?");
    gtk_alert_dialog_set_detail(dialog, "This will reset all displays to 0 vibrance (normal colors).");
    gtk_alert_dialog_set_buttons(dialog, (const char*[]){"Cancel", "Reset", nullptr});
    gtk_alert_dialog_set_cancel_button(dialog, 0);
    gtk_alert_dialog_set_default_button(dialog, 1);
    
    gtk_alert_dialog_choose(dialog, GTK_WINDOW(window->m_window), nullptr, 
        [](GObject* source, GAsyncResult* result, gpointer user_data) {
            auto* window = static_cast<MainWindow*>(user_data);
            GtkAlertDialog* dialog = GTK_ALERT_DIALOG(source);
            
            int response = gtk_alert_dialog_choose_finish(dialog, result, nullptr);
            if (response == 1) { // Reset button
                // Reset all displays
                for (const auto& pair : window->m_vibranceScales) {
                    gtk_range_set_value(GTK_RANGE(pair.second), 0.0);
                    window->m_manager->resetVibrance(pair.first);
                }
                std::cout << "🔄 All displays reset to normal vibrance" << std::endl;
            }
            g_object_unref(dialog);
        }, window);
}

void MainWindow::onAddProgramClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    window->showFileBrowserDialog();
}

void MainWindow::showFileBrowserDialog() {
    // Use modern GTK4 file dialog
    GtkFileDialog* dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Select Program to Add");
    
    // Set up file filters
    GtkFileFilter* executableFilter = gtk_file_filter_new();
    gtk_file_filter_set_name(executableFilter, "Executable Files");
    gtk_file_filter_add_mime_type(executableFilter, "application/x-executable");
    gtk_file_filter_add_pattern(executableFilter, "*.exe");
    gtk_file_filter_add_pattern(executableFilter, "*.AppImage");
    
    GtkFileFilter* allFilter = gtk_file_filter_new();
    gtk_file_filter_set_name(allFilter, "All Files");
    gtk_file_filter_add_pattern(allFilter, "*");
    
    GListStore* filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    g_list_store_append(filters, executableFilter);
    g_list_store_append(filters, allFilter);
    gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));
    
    // Set initial folder to common executable locations
    GFile* initialFolder = g_file_new_for_path("/usr/bin");
    gtk_file_dialog_set_initial_folder(dialog, initialFolder);
    g_object_unref(initialFolder);
    
    gtk_file_dialog_open(dialog, GTK_WINDOW(m_window), nullptr,
        [](GObject* source, GAsyncResult* result, gpointer user_data) {
            auto* window = static_cast<MainWindow*>(user_data);
            GtkFileDialog* dialog = GTK_FILE_DIALOG(source);
            
            GFile* file = gtk_file_dialog_open_finish(dialog, result, nullptr);
            if (file) {
                char* path = g_file_get_path(file);
                if (path) {
                    window->showProgramConfigDialog(std::string(path));
                    g_free(path);
                }
                g_object_unref(file);
            }
            g_object_unref(dialog);
        }, this);
    
    g_object_unref(filters);
    g_object_unref(executableFilter);
    g_object_unref(allFilter);
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
    
    // Use modern GTK4 entry buffer
    GtkEntryBuffer* buffer = gtk_entry_buffer_new(programName.c_str(), -1);
    gtk_entry_set_buffer(GTK_ENTRY(nameEntry), buffer);
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
    g_object_unref(buffer);
}

void MainWindow::onSaveProgramProfile(GtkButton* button, gpointer user_data __attribute__((unused))) {
    const char* programPath = static_cast<const char*>(g_object_get_data(G_OBJECT(button), "program-path"));
    GtkEntry* nameEntry = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "name-entry"));
    GtkWidget* dialog = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "dialog"));
    MainWindow* window = static_cast<MainWindow*>(g_object_get_data(G_OBJECT(button), "main-window"));
    
    if (programPath && nameEntry && window) {
        AppProfile profile;
        
        // Get text from entry buffer
        GtkEntryBuffer* buffer = gtk_entry_get_buffer(nameEntry);
        profile.name = gtk_entry_buffer_get_text(buffer);
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

void MainWindow::onAutostartToggled(GtkCheckButton* button, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    bool enabled = gtk_check_button_get_active(button);
    
    std::cout << "🚀 Autostart toggle: " << (enabled ? "enabling" : "disabling") << std::endl;
    
    bool success = false;
    if (enabled) {
        success = window->m_manager->enableAutostart();
        if (success) {
            std::cout << "✅ Autostart enabled successfully" << std::endl;
        } else {
            std::cout << "❌ Failed to enable autostart" << std::endl;
            // Revert checkbox state
            gtk_check_button_set_active(button, FALSE);
        }
    } else {
        success = window->m_manager->disableAutostart();
        if (success) {
            std::cout << "✅ Autostart disabled successfully" << std::endl;
        } else {
            std::cout << "❌ Failed to disable autostart" << std::endl;
            // Revert checkbox state
            gtk_check_button_set_active(button, TRUE);
        }
    }
    
    // Update status display
    window->updateAutostartStatus();
    
    if (success) {
        // Show success notification using modern GTK4 dialog
        GtkAlertDialog* dialog = gtk_alert_dialog_new(enabled ? "Autostart Enabled" : "Autostart Disabled");
        gtk_alert_dialog_set_detail(dialog, enabled ? 
            "Vivid will now start automatically when you log in." :
            "Vivid will no longer start automatically.");
        gtk_alert_dialog_set_buttons(dialog, (const char*[]){"OK", nullptr});
        
        gtk_alert_dialog_show(dialog, GTK_WINDOW(window->m_window));
        g_object_unref(dialog);
    }
}

void MainWindow::onAutostartDebugClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    window->showAutostartDebugDialog();
}

void MainWindow::showAutostartDebugDialog() {
    GtkWidget* dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Autostart Debug Information");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(m_window));
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 500);
    
    // Main container
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_window_set_child(GTK_WINDOW(dialog), vbox);
    
    // Title
    GtkWidget* title = gtk_label_new("🔧 Autostart Debug Information");
    gtk_label_set_markup(GTK_LABEL(title), 
        "<span size='large' weight='bold'>🔧 Autostart Debug Information</span>");
    
    // Scrolled text view for debug info
    GtkWidget* scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    GtkWidget* textView = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textView), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(textView), TRUE);
    gtk_widget_add_css_class(textView, "program-list");
    
    // Get debug info from autostart manager
    std::string debugInfo = "=== Autostart Debug Information ===\n\n";
    debugInfo += "Status: " + m_manager->getAutostartStatus() + "\n";
    debugInfo += "This feature provides detailed debugging information\n";
    debugInfo += "about the autostart configuration and any issues.\n\n";
    debugInfo += "Current session: " + std::string(std::getenv("XDG_SESSION_TYPE") ?: "unknown") + "\n";
    debugInfo += "Desktop environment: " + std::string(std::getenv("XDG_CURRENT_DESKTOP") ?: "unknown") + "\n";
    debugInfo += "Home directory: " + std::string(std::getenv("HOME") ?: "unknown") + "\n";
    
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
    gtk_text_buffer_set_text(buffer, debugInfo.c_str(), -1);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), textView);
    
    // Buttons
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(buttonBox, GTK_ALIGN_END);
    
    GtkWidget* copyButton = gtk_button_new_with_label("📋 Copy to Clipboard");
    gtk_widget_add_css_class(copyButton, "modern-button");
    
    GtkWidget* closeButton = gtk_button_new_with_label("Close");
    gtk_widget_add_css_class(closeButton, "modern-button");
    
    gtk_box_append(GTK_BOX(buttonBox), copyButton);
    gtk_box_append(GTK_BOX(buttonBox), closeButton);
    
    // Assemble dialog
    gtk_box_append(GTK_BOX(vbox), title);
    gtk_box_append(GTK_BOX(vbox), scrolled);
    gtk_box_append(GTK_BOX(vbox), buttonBox);
    
    // Connect signals
    g_signal_connect_swapped(closeButton, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    
    gtk_window_present(GTK_WINDOW(dialog));
}

void MainWindow::updateAutostartStatus() {
    if (m_autostartStatusLabel) {
        std::string status = m_manager->getAutostartStatus();
        gtk_label_set_text(GTK_LABEL(m_autostartStatusLabel), status.c_str());
    }
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
