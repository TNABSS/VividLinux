#include "MainWindow.h"
#include <iostream>
#include <filesystem>
#include <functional>

MainWindow::MainWindow(GtkApplication* app) {
    m_controller = std::make_unique<VibranceController>();
    
    m_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(m_window), "Vivid - Digital Vibrance Control");
    gtk_window_set_default_size(GTK_WINDOW(m_window), 450, 350);
    gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);
    
    applyGrayTheme();
    setupUI();
}

MainWindow::~MainWindow() = default;

void MainWindow::applyGrayTheme() {
    GtkCssProvider* provider = gtk_css_provider_new();
    
    // FIXED: Removed invalid CSS properties
    const char* css = R"(
        window {
            background-color: #2d2d2d;
            color: #ffffff;
            font-family: 'Segoe UI', sans-serif;
        }
        
        .main-container {
            background-color: #2d2d2d;
            padding: 20px;
        }
        
        .display-section {
            background-color: #3a3a3a;
            border: 1px solid #555555;
            border-radius: 8px;
            padding: 15px;
            margin: 10px 0;
        }
        
        .display-title {
            font-size: 14px;
            font-weight: bold;
            color: #ffffff;
            margin-bottom: 10px;
        }
        
        .value-label {
            font-size: 18px;
            font-weight: bold;
            color: #cccccc;
            margin: 8px 0;
        }
        
        .vibrance-info {
            font-size: 12px;
            color: #999999;
            margin: 5px 0;
        }
        
        scale {
            margin: 15px 0;
        }
        
        scale trough {
            background: linear-gradient(to right, #ff4444 0%, #555555 50%, #44ff44 100%);
            border-radius: 4px;
            min-height: 8px;
        }
        
        scale slider {
            background-color: #ffffff;
            border: 2px solid #888888;
            border-radius: 50%;
            min-width: 18px;
            min-height: 18px;
        }
        
        scale slider:hover {
            background-color: #f0f0f0;
            border-color: #aaaaaa;
        }
        
        button {
            background-color: #4a4a4a;
            color: #ffffff;
            border: 1px solid #666666;
            border-radius: 6px;
            padding: 12px 24px;
            margin: 5px;
            font-weight: 500;
            min-height: 44px;
        }
        
        button:hover {
            background-color: #555555;
            border-color: #777777;
        }
        
        button:active {
            background-color: #3a3a3a;
        }
        
        .apply-button {
            background-color: #0066cc;
            font-weight: bold;
            min-width: 140px;
        }
        
        .apply-button:hover {
            background-color: #0077dd;
        }
        
        .install-button {
            background-color: #006600;
        }
        
        .install-button:hover {
            background-color: #007700;
        }
        
        .reset-button {
            background-color: #cc6600;
        }
        
        .reset-button:hover {
            background-color: #dd7700;
        }
        
        .button-box {
            margin-top: 20px;
            padding-top: 15px;
            border-top: 1px solid #555555;
        }
        
        .status-info {
            background-color: #333333;
            border: 1px solid #555555;
            border-radius: 4px;
            padding: 10px;
            margin: 10px 0;
            font-size: 12px;
            color: #cccccc;
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
    m_mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(m_mainBox, "main-container");
    gtk_window_set_child(GTK_WINDOW(m_window), m_mainBox);
    
    // Add status info
    GtkWidget* statusInfo = gtk_label_new("🎮 Move sliders to see REAL vibrance changes!\n💡 Changes are applied instantly and persist until reset.");
    gtk_widget_add_css_class(statusInfo, "status-info");
    gtk_label_set_wrap(GTK_LABEL(statusInfo), TRUE);
    gtk_box_append(GTK_BOX(m_mainBox), statusInfo);
    
    setupDisplayControls();
    
    // Button section
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(buttonBox, "button-box");
    gtk_widget_set_halign(buttonBox, GTK_ALIGN_CENTER);
    
    m_applyButton = gtk_button_new_with_label("💾 Save Settings");
    gtk_widget_add_css_class(m_applyButton, "apply-button");
    g_signal_connect(m_applyButton, "clicked", G_CALLBACK(onApplyClicked), this);
    
    m_installButton = gtk_button_new_with_label("🌐 Install System-Wide");
    gtk_widget_add_css_class(m_installButton, "install-button");
    g_signal_connect(m_installButton, "clicked", G_CALLBACK(onInstallClicked), this);
    
    m_resetButton = gtk_button_new_with_label("🔄 Reset All");
    gtk_widget_add_css_class(m_resetButton, "reset-button");
    g_signal_connect(m_resetButton, "clicked", G_CALLBACK(onResetClicked), this);
    
    gtk_box_append(GTK_BOX(buttonBox), m_applyButton);
    gtk_box_append(GTK_BOX(buttonBox), m_installButton);
    gtk_box_append(GTK_BOX(buttonBox), m_resetButton);
    
    gtk_box_append(GTK_BOX(m_mainBox), buttonBox);
}

void MainWindow::setupDisplayControls() {
    auto displays = m_controller->getDisplays();
    
    for (const auto& display : displays) {
        GtkWidget* displaySection = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        gtk_widget_add_css_class(displaySection, "display-section");
        
        std::string titleText = "🖥️ Display: " + display.id;
        GtkWidget* titleLabel = gtk_label_new(titleText.c_str());
        gtk_widget_add_css_class(titleLabel, "display-title");
        gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
        
        GtkWidget* valueLabel = gtk_label_new("Normal (0%)");
        gtk_widget_add_css_class(valueLabel, "value-label");
        gtk_widget_set_halign(valueLabel, GTK_ALIGN_CENTER);
        m_valueLabels[display.id] = valueLabel;
        
        // Info label
        GtkWidget* infoLabel = gtk_label_new("← Less Saturated | More Saturated →");
        gtk_widget_add_css_class(infoLabel, "vibrance-info");
        gtk_widget_set_halign(infoLabel, GTK_ALIGN_CENTER);
        
        GtkWidget* vibranceScale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -100.0, 100.0, 1.0);
        gtk_scale_set_draw_value(GTK_SCALE(vibranceScale), FALSE);
        gtk_range_set_value(GTK_RANGE(vibranceScale), display.currentVibrance);
        
        // More visible marks
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), -100.0, GTK_POS_BOTTOM, "Grayscale");
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), -50.0, GTK_POS_BOTTOM, "-50%");
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), 0.0, GTK_POS_BOTTOM, "Normal");
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), 50.0, GTK_POS_BOTTOM, "+50%");
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), 100.0, GTK_POS_BOTTOM, "Vivid");
        
        m_vibranceScales[display.id] = vibranceScale;
        g_object_set_data(G_OBJECT(vibranceScale), "display_id", g_strdup(display.id.c_str()));
        g_signal_connect(vibranceScale, "value-changed", G_CALLBACK(onVibranceChanged), this);
        
        gtk_box_append(GTK_BOX(displaySection), titleLabel);
        gtk_box_append(GTK_BOX(displaySection), valueLabel);
        gtk_box_append(GTK_BOX(displaySection), infoLabel);
        gtk_box_append(GTK_BOX(displaySection), vibranceScale);
        
        gtk_box_append(GTK_BOX(m_mainBox), displaySection);
    }
}

void MainWindow::updateValueLabel(const std::string& displayId, int vibrance) {
    auto it = m_valueLabels.find(displayId);
    if (it != m_valueLabels.end()) {
        std::string text;
        if (vibrance == 0) {
            text = "Normal (0%)";
        } else if (vibrance > 0) {
            text = "Enhanced (+" + std::to_string(vibrance) + "%)";
        } else {
            text = "Reduced (" + std::to_string(vibrance) + "%)";
        }
        
        gtk_label_set_text(GTK_LABEL(it->second), text.c_str());
    }
}

// Event handlers
void MainWindow::onVibranceChanged(GtkRange* range, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    const char* displayId = static_cast<const char*>(g_object_get_data(G_OBJECT(range), "display_id"));
    if (!displayId) return;
    
    double value = gtk_range_get_value(range);
    int vibrance = static_cast<int>(value);
    
    window->m_controller->setVibrance(displayId, vibrance);
    window->updateValueLabel(displayId, vibrance);
}

void MainWindow::onApplyClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    window->showConfirmDialog(
        "💾 Save current vibrance settings?\n\nThis will make the current settings persistent across reboots.",
        [window]() {
            window->applyAllSettings();
            window->showInfoDialog("✅ Settings saved successfully!\n\n🎮 Your vibrance settings are now persistent.");
        }
    );
}

void MainWindow::onInstallClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    if (window->m_controller->isSystemInstalled()) {
        window->showInfoDialog("✅ Vivid is already installed system-wide!\n\nYou can run 'vivid' from anywhere.");
        return;
    }
    
    window->showConfirmDialog(
        "🌐 Install Vivid system-wide?\n\nThis will:\n• Make 'vivid' available from terminal\n• Add Vivid to applications menu\n• Require admin password",
        [window]() {
            if (window->m_controller->installSystemWide()) {
                window->showInfoDialog("✅ Vivid installed system-wide!\n\n🚀 You can now:\n• Run 'vivid' from terminal\n• Find Vivid in applications menu\n• Use CLI commands anywhere");
            } else {
                window->showInfoDialog("❌ Installation failed.\n\n💡 Try running:\nsudo cp builddir/vivid /usr/local/bin/");
            }
        }
    );
}

void MainWindow::onResetClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    window->showConfirmDialog(
        "🔄 Reset all displays to normal vibrance?\n\nThis will set all values to 0% (normal colors).",
        [window]() {
            window->m_controller->resetAllDisplays();
            
            for (const auto& pair : window->m_vibranceScales) {
                gtk_range_set_value(GTK_RANGE(pair.second), 0.0);
                window->updateValueLabel(pair.first, 0);
            }
            
            window->showInfoDialog("✅ All displays reset to normal!\n\n🎨 Colors restored to default values.");
        }
    );
}

void MainWindow::applyAllSettings() {
    for (const auto& pair : m_vibranceScales) {
        double value = gtk_range_get_value(GTK_RANGE(pair.second));
        int vibrance = static_cast<int>(value);
        m_controller->setVibrance(pair.first, vibrance);
    }
}

// FIXED: Proper GTK callback functions instead of lambdas
static void on_cancel_clicked(GtkButton* button, gpointer user_data) {
    gtk_window_destroy(GTK_WINDOW(user_data));
}

static void on_ok_clicked(GtkButton* button, gpointer user_data) {
    auto* callbackPtr = static_cast<std::function<void()>*>(g_object_get_data(G_OBJECT(user_data), "callback"));
    if (callbackPtr) (*callbackPtr)();
    gtk_window_destroy(GTK_WINDOW(user_data));
}

static void on_info_ok_clicked(GtkButton* button, gpointer user_data) {
    gtk_window_destroy(GTK_WINDOW(user_data));
}

void MainWindow::showConfirmDialog(const std::string& message, std::function<void()> callback) {
    GtkWidget* dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Confirm Action");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(m_window));
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 180);
    
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    
    GtkWidget* label = gtk_label_new(message.c_str());
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_halign(buttonBox, GTK_ALIGN_CENTER);
    
    GtkWidget* cancelBtn = gtk_button_new_with_label("❌ Cancel");
    GtkWidget* okBtn = gtk_button_new_with_label("✅ Confirm");
    
    gtk_box_append(GTK_BOX(buttonBox), cancelBtn);
    gtk_box_append(GTK_BOX(buttonBox), okBtn);
    
    gtk_box_append(GTK_BOX(box), label);
    gtk_box_append(GTK_BOX(box), buttonBox);
    
    gtk_window_set_child(GTK_WINDOW(dialog), box);
    
    // Store callback
    auto* callbackPtr = new std::function<void()>(callback);
    g_object_set_data_full(G_OBJECT(dialog), "callback", callbackPtr, [](gpointer data) {
        delete static_cast<std::function<void()>*>(data);
    });
    
    g_signal_connect(cancelBtn, "clicked", G_CALLBACK(on_cancel_clicked), dialog);
    g_signal_connect(okBtn, "clicked", G_CALLBACK(on_ok_clicked), dialog);
    
    gtk_window_present(GTK_WINDOW(dialog));
}

void MainWindow::showInfoDialog(const std::string& message) {
    GtkWidget* dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Information");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(m_window));
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 160);
    
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    
    GtkWidget* label = gtk_label_new(message.c_str());
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    
    GtkWidget* okBtn = gtk_button_new_with_label("✅ OK");
    gtk_widget_set_halign(okBtn, GTK_ALIGN_CENTER);
    
    gtk_box_append(GTK_BOX(box), label);
    gtk_box_append(GTK_BOX(box), okBtn);
    
    gtk_window_set_child(GTK_WINDOW(dialog), box);
    
    g_signal_connect(okBtn, "clicked", G_CALLBACK(on_info_ok_clicked), dialog);
    
    gtk_window_present(GTK_WINDOW(dialog));
}

void MainWindow::show() {
    gtk_window_present(GTK_WINDOW(m_window));
}
