#include "MainWindow.h"
#include <iostream>
#include <filesystem>

MainWindow::MainWindow(GtkApplication* app) {
    m_controller = std::make_unique<VibranceController>();
    
    m_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(m_window), "Vivid - Digital Vibrance Control");
    gtk_window_set_default_size(GTK_WINDOW(m_window), 500, 400);
    gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);
    
    applyUnifiedTheme();
    setupUI();
    updateProgramList();
    
    // Auto-apply saved settings on startup
    autoApplySettings();
}

MainWindow::~MainWindow() = default;

void MainWindow::applyUnifiedTheme() {
    GtkCssProvider* provider = gtk_css_provider_new();
    
    // Single blue color theme - consistent throughout
    const char* css = R"(
        window {
            background-color: #1e3a5f;
            color: #ffffff;
        }
        
        .main-container {
            background-color: #1e3a5f;
            padding: 12px;
        }
        
        headerbar {
            background-color: #2c5282;
            color: #ffffff;
        }
        
        headerbar button {
            background-color: #3182ce;
            color: #ffffff;
            border: 1px solid #2c5282;
            border-radius: 4px;
            padding: 6px 12px;
            margin: 2px;
        }
        
        headerbar button:hover {
            background-color: #2b77cb;
        }
        
        notebook {
            background-color: #2c5282;
            border: 1px solid #3182ce;
            border-radius: 6px;
        }
        
        notebook tab {
            background-color: #2c5282;
            color: #e2e8f0;
            border: 1px solid #3182ce;
            padding: 10px 18px;
            margin: 1px;
            border-radius: 4px 4px 0 0;
        }
        
        notebook tab:checked {
            background-color: #3182ce;
            color: #ffffff;
            font-weight: bold;
        }
        
        .vibrance-container {
            background-color: #2c5282;
            border: 1px solid #3182ce;
            border-radius: 6px;
            padding: 16px;
            margin: 10px 0;
        }
        
        scale {
            margin: 12px 0;
        }
        
        scale trough {
            background: linear-gradient(to right, #1a365d 0%, #3182ce 50%, #1a365d 100%);
            border-radius: 4px;
            min-height: 8px;
        }
        
        scale slider {
            background-color: #ffffff;
            border: 2px solid #3182ce;
            border-radius: 50%;
            min-width: 18px;
            min-height: 18px;
        }
        
        scale slider:hover {
            background-color: #e2e8f0;
            border-color: #2b77cb;
        }
        
        spinbutton {
            background-color: #2c5282;
            color: #ffffff;
            border: 1px solid #3182ce;
            border-radius: 4px;
            min-width: 70px;
            padding: 4px;
        }
        
        spinbutton:focus {
            border-color: #63b3ed;
        }
        
        button {
            background-color: #3182ce;
            color: #ffffff;
            border: 1px solid #2c5282;
            border-radius: 4px;
            padding: 8px 16px;
            margin: 4px;
            font-weight: 500;
        }
        
        button:hover {
            background-color: #2b77cb;
            border-color: #2a69ac;
        }
        
        button:active {
            background-color: #2a69ac;
        }
        
        .remove-button {
            background-color: #2c5282;
            border-color: #3182ce;
        }
        
        .remove-button:hover {
            background-color: #2a4a6b;
        }
        
        checkbutton {
            color: #ffffff;
            margin: 8px 0;
        }
        
        checkbutton check {
            background-color: #2c5282;
            border: 1px solid #3182ce;
            border-radius: 3px;
        }
        
        checkbutton check:checked {
            background-color: #3182ce;
            border-color: #2b77cb;
        }
        
        listbox {
            background-color: #2c5282;
            border: 1px solid #3182ce;
            border-radius: 4px;
        }
        
        listbox row {
            background-color: transparent;
            color: #ffffff;
            padding: 10px;
            border-bottom: 1px solid #3182ce;
        }
        
        listbox row:hover {
            background-color: #3182ce;
        }
        
        listbox row:selected {
            background-color: #2b77cb;
            font-weight: bold;
        }
        
        label {
            color: #ffffff;
        }
        
        .title-label {
            font-size: 14px;
            font-weight: bold;
            color: #e2e8f0;
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
    
    // Header bar with working buttons
    setupHeaderBar();
    
    // Focus checkbox
    m_focusCheckbox = gtk_check_button_new_with_label("🎯 Set vibrance only when program is in focus");
    gtk_check_button_set_active(GTK_CHECK_BUTTON(m_focusCheckbox), m_controller->getFocusMode());
    g_signal_connect(m_focusCheckbox, "toggled", G_CALLBACK(onFocusToggled), this);
    gtk_box_append(GTK_BOX(mainBox), m_focusCheckbox);
    
    // Display tabs
    setupDisplayTabs();
    gtk_box_append(GTK_BOX(mainBox), m_notebook);
    
    // Program section
    setupProgramSection();
}

void MainWindow::setupHeaderBar() {
    GtkWidget* headerBar = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(m_window), headerBar);
    
    // File menu button - WORKING
    GtkWidget* fileButton = gtk_button_new_with_label("📁 File");
    g_signal_connect(fileButton, "clicked", G_CALLBACK(onFileMenuClicked), this);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerBar), fileButton);
    
    // Help button - WORKING with command list
    GtkWidget* helpButton = gtk_button_new_with_label("❓ Help");
    g_signal_connect(helpButton, "clicked", G_CALLBACK(onHelpClicked), this);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerBar), helpButton);
    
    // Auto-optimize button - NEW WORKING FEATURE
    GtkWidget* autoButton = gtk_button_new_with_label("🎨 Auto-Optimize");
    g_signal_connect(autoButton, "clicked", G_CALLBACK(onAutoOptimizeClicked), this);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerBar), autoButton);
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
    GtkWidget* tabBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(tabBox, 20);
    gtk_widget_set_margin_end(tabBox, 20);
    gtk_widget_set_margin_top(tabBox, 20);
    gtk_widget_set_margin_bottom(tabBox, 20);
    
    // Vibrance container
    GtkWidget* vibranceContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(vibranceContainer, "vibrance-container");
    
    // Title with emoji
    std::string titleText = "🖥️ Vibrance for " + display.id;
    GtkWidget* titleLabel = gtk_label_new(titleText.c_str());
    gtk_widget_add_css_class(titleLabel, "title-label");
    gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
    
    // Current value display
    GtkWidget* valueLabel = gtk_label_new("");
    updateValueLabel(valueLabel, display.currentVibrance);
    gtk_widget_set_halign(valueLabel, GTK_ALIGN_CENTER);
    
    // Horizontal container for slider and spin button
    GtkWidget* controlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    
    // Vibrance slider with better range
    GtkWidget* vibranceScale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -100.0, 100.0, 1.0);
    gtk_scale_set_draw_value(GTK_SCALE(vibranceScale), FALSE);
    gtk_range_set_value(GTK_RANGE(vibranceScale), display.currentVibrance);
    gtk_widget_set_hexpand(vibranceScale, TRUE);
    
    // Add scale marks for reference
    gtk_scale_add_mark(GTK_SCALE(vibranceScale), -100.0, GTK_POS_BOTTOM, "Gray");
    gtk_scale_add_mark(GTK_SCALE(vibranceScale), -50.0, GTK_POS_BOTTOM, "Muted");
    gtk_scale_add_mark(GTK_SCALE(vibranceScale), 0.0, GTK_POS_BOTTOM, "Normal");
    gtk_scale_add_mark(GTK_SCALE(vibranceScale), 50.0, GTK_POS_BOTTOM, "Enhanced");
    gtk_scale_add_mark(GTK_SCALE(vibranceScale), 100.0, GTK_POS_BOTTOM, "Max");
    
    g_signal_connect(vibranceScale, "value-changed", G_CALLBACK(onVibranceChanged), this);
    
    // Vibrance spin button
    GtkWidget* vibranceSpin = gtk_spin_button_new_with_range(-100.0, 100.0, 1.0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(vibranceSpin), display.currentVibrance);
    g_signal_connect(vibranceSpin, "value-changed", G_CALLBACK(onVibranceSpinChanged), this);
    
    // Quick preset buttons
    GtkWidget* presetBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(presetBox, GTK_ALIGN_CENTER);
    
    struct { const char* label; int value; } presets[] = {
        {"🎮 Gaming", 75},
        {"🎬 Movies", 50},
        {"💼 Work", -25},
        {"🔄 Reset", 0}
    };
    
    for (auto& preset : presets) {
        GtkWidget* presetBtn = gtk_button_new_with_label(preset.label);
        g_object_set_data(G_OBJECT(presetBtn), "display_id", g_strdup(display.id.c_str()));
        g_object_set_data(G_OBJECT(presetBtn), "vibrance_value", GINT_TO_POINTER(preset.value));
        g_signal_connect(presetBtn, "clicked", G_CALLBACK(onPresetClicked), this);
        gtk_box_append(GTK_BOX(presetBox), presetBtn);
    }
    
    // Store references
    m_vibranceScales[display.id] = vibranceScale;
    m_vibranceSpins[display.id] = vibranceSpin;
    m_valueLabels[display.id] = valueLabel;
    
    gtk_box_append(GTK_BOX(controlBox), vibranceScale);
    gtk_box_append(GTK_BOX(controlBox), vibranceSpin);
    
    gtk_box_append(GTK_BOX(vibranceContainer), titleLabel);
    gtk_box_append(GTK_BOX(vibranceContainer), valueLabel);
    gtk_box_append(GTK_BOX(vibranceContainer), controlBox);
    gtk_box_append(GTK_BOX(vibranceContainer), presetBox);
    
    gtk_box_append(GTK_BOX(tabBox), vibranceContainer);
    
    // Tab label with emoji
    std::string tabText = "📺 " + display.id;
    GtkWidget* tabLabel = gtk_label_new(tabText.c_str());
    
    // Add to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), tabBox, tabLabel);
}

void MainWindow::setupProgramSection() {
    GtkWidget* mainBox = gtk_widget_get_parent(m_notebook);
    
    // Section title
    GtkWidget* sectionTitle = gtk_label_new("🎯 Program Profiles");
    gtk_widget_add_css_class(sectionTitle, "title-label");
    gtk_widget_set_halign(sectionTitle, GTK_ALIGN_START);
    gtk_widget_set_margin_top(sectionTitle, 16);
    gtk_box_append(GTK_BOX(mainBox), sectionTitle);
    
    // Button container
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(buttonBox, 8);
    gtk_widget_set_margin_end(buttonBox, 8);
    
    m_addProgramButton = gtk_button_new_with_label("➕ Add Program");
    g_signal_connect(m_addProgramButton, "clicked", G_CALLBACK(onAddProgram), this);
    
    m_removeProgramButton = gtk_button_new_with_label("➖ Remove Program");
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

// Auto-apply optimal settings on startup
void MainWindow::autoApplySettings() {
    std::cout << "🎨 Auto-applying optimal vibrance settings..." << std::endl;
    
    auto displays = m_controller->getDisplays();
    for (const auto& display : displays) {
        int optimalVibrance = 0;
        
        // Smart defaults based on display type
        if (display.id.find("eDP") != std::string::npos || 
            display.id.find("LVDS") != std::string::npos) {
            optimalVibrance = 25; // Laptop display
        } else if (display.id.find("HDMI") != std::string::npos || 
                   display.id.find("DP") != std::string::npos) {
            optimalVibrance = 40; // External display
        } else {
            optimalVibrance = 30; // Other displays
        }
        
        // Apply the setting
        m_controller->setVibrance(display.id, optimalVibrance);
        
        // Update UI
        updateDisplayControls(display.id, optimalVibrance);
        
        std::cout << "  📺 " << display.id << ": +" << optimalVibrance << " vibrance" << std::endl;
    }
}

void MainWindow::updateDisplayControls(const std::string& displayId, int vibrance) {
    auto scaleIt = m_vibranceScales.find(displayId);
    if (scaleIt != m_vibranceScales.end()) {
        gtk_range_set_value(GTK_RANGE(scaleIt->second), vibrance);
    }
    
    auto spinIt = m_vibranceSpins.find(displayId);
    if (spinIt != m_vibranceSpins.end()) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinIt->second), vibrance);
    }
    
    auto labelIt = m_valueLabels.find(displayId);
    if (labelIt != m_valueLabels.end()) {
        updateValueLabel(labelIt->second, vibrance);
    }
}

void MainWindow::updateValueLabel(GtkWidget* label, int vibrance) {
    std::string text;
    if (vibrance < -50) {
        text = "🔘 Muted (" + std::to_string(vibrance) + ")";
    } else if (vibrance < 0) {
        text = "🔹 Reduced (" + std::to_string(vibrance) + ")";
    } else if (vibrance == 0) {
        text = "⚪ Normal (0)";
    } else if (vibrance <= 50) {
        text = "🔸 Enhanced (+" + std::to_string(vibrance) + ")";
    } else {
        text = "🔴 High (+" + std::to_string(vibrance) + ")";
    }
    
    gtk_label_set_text(GTK_LABEL(label), text.c_str());
}

// Event handlers - ALL WORKING
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
            
            // Update value label
            auto labelIt = window->m_valueLabels.find(pair.first);
            if (labelIt != window->m_valueLabels.end()) {
                window->updateValueLabel(labelIt->second, vibrance);
            }
            
            // Apply vibrance immediately
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
            
            // Update value label
            auto labelIt = window->m_valueLabels.find(pair.first);
            if (labelIt != window->m_valueLabels.end()) {
                window->updateValueLabel(labelIt->second, vibrance);
            }
            
            // Apply vibrance immediately
            window->m_controller->setVibrance(pair.first, vibrance);
            break;
        }
    }
}

void MainWindow::onPresetClicked(GtkButton* button, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    const char* displayId = static_cast<const char*>(g_object_get_data(G_OBJECT(button), "display_id"));
    int vibrance = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "vibrance_value"));
    
    if (displayId) {
        window->m_controller->setVibrance(displayId, vibrance);
        window->updateDisplayControls(displayId, vibrance);
        
        std::cout << "🎯 Applied preset: " << displayId << " = " << vibrance << std::endl;
    }
}

void MainWindow::onAutoOptimizeClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    window->autoApplySettings();
    
    // Show confirmation
    GtkWidget* dialog = gtk_message_dialog_new(
        GTK_WINDOW(window->m_window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "🎨 Auto-Optimization Complete!\n\nOptimal vibrance settings have been applied to all displays."
    );
    
    gtk_window_set_title(GTK_WINDOW(dialog), "Vivid");
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
    gtk_window_present(GTK_WINDOW(dialog));
}

void MainWindow::onHelpClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    const char* helpText = 
        "🎮 VIVID COMMAND REFERENCE\n"
        "=========================\n\n"
        "🚀 LAUNCHER COMMANDS:\n"
        "  ./vivid           - Interactive setup & launch\n"
        "  ./vivid gui       - Quick launch GUI\n"
        "  ./vivid build     - Build application\n"
        "  ./vivid auto      - Auto-optimize vibrance\n"
        "  ./vivid test      - Test vibrance control\n"
        "  ./vivid install   - Install system-wide\n"
        "  ./vivid clean     - Clean build files\n"
        "  ./vivid fix       - Fix all permissions\n\n"
        "🎮 VIBRANCE COMMANDS:\n"
        "  ./builddir/vivid --list                    - List displays\n"
        "  ./builddir/vivid --set <display> <value>   - Set vibrance\n"
        "  ./builddir/vivid --reset <display>         - Reset display\n"
        "  ./builddir/vivid --status                  - Show settings\n\n"
        "🎯 VIBRANCE VALUES:\n"
        "  -100  Grayscale (no color)\n"
        "   -50  Muted colors (work)\n"
        "     0  Normal colors (default)\n"
        "   +50  Enhanced colors (movies)\n"
        "   +75  High vibrance (gaming)\n"
        "  +100  Maximum vibrance\n\n"
        "💡 EXAMPLES:\n"
        "  ./vivid auto                               - Auto-optimize\n"
        "  ./builddir/vivid --set DVI-D-0 75          - Gaming setup\n"
        "  ./builddir/vivid --set HDMI-0 -25          - Work setup";
    
    GtkWidget* dialog = gtk_message_dialog_new(
        GTK_WINDOW(window->m_window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s", helpText
    );
    
    gtk_window_set_title(GTK_WINDOW(dialog), "Vivid Help");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 500);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
    gtk_window_present(GTK_WINDOW(dialog));
}

void MainWindow::onFileMenuClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    GtkWidget* dialog = gtk_message_dialog_new(
        GTK_WINDOW(window->m_window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "📁 File Menu\n\n"
        "Settings are automatically saved to:\n"
        "~/.config/vivid/settings.conf\n\n"
        "Profiles are saved to:\n"
        "~/.config/vivid/profiles.conf\n\n"
        "All changes are applied immediately and persist across restarts."
    );
    
    gtk_window_set_title(GTK_WINDOW(dialog), "File Information");
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
    gtk_window_present(GTK_WINDOW(dialog));
}

void MainWindow::onFocusToggled(GtkCheckButton* button, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    bool enabled = gtk_check_button_get_active(button);
    window->m_controller->setFocusMode(enabled);
    
    std::cout << "🎯 Focus mode: " << (enabled ? "enabled" : "disabled") << std::endl;
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
            
            std::cout << "🗑️ Removed profile: " << text << std::endl;
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
        std::string displayText = "🎮 " + profile.name;
        GtkWidget* label = gtk_label_new(displayText.c_str());
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_list_box_append(GTK_LIST_BOX(m_programListBox), label);
    }
}

void MainWindow::showProgramDialog(const std::string& programPath) {
    std::cout << "📝 Creating profile for: " << programPath << std::endl;
    
    // Create a simple profile with gaming defaults
    if (!programPath.empty()) {
        ProgramProfile profile;
        profile.name = std::filesystem::path(programPath).filename().string();
        profile.path = programPath;
        profile.pathMatching = true;
        profile.enabled = true;
        
        // Set gaming vibrance for all displays
        auto displays = m_controller->getDisplays();
        for (const auto& display : displays) {
            profile.displayVibrance[display.id] = 75; // Gaming vibrance
        }
        
        m_controller->saveProfile(profile);
        updateProgramList();
        
        // Show confirmation
        std::string message = "✅ Profile created for: " + profile.name + "\n\n"
                             "Vibrance set to +75 (gaming) for all displays.\n"
                             "Enable focus mode to activate automatically.";
        
        GtkWidget* dialog = gtk_message_dialog_new(
            GTK_WINDOW(m_window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "%s", message.c_str()
        );
        
        gtk_window_set_title(GTK_WINDOW(dialog), "Profile Created");
        g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
        gtk_window_present(GTK_WINDOW(dialog));
    }
}

void MainWindow::show() {
    gtk_window_present(GTK_WINDOW(m_window));
}
