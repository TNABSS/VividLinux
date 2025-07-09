#include "MainWindow.h"
#include <iostream>
#include <filesystem>
#include <functional>  // FIXED: Added missing include

MainWindow::MainWindow(GtkApplication* app) {
    m_controller = std::make_unique<VibranceController>();
    
    m_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(m_window), "Vivid");
    gtk_window_set_default_size(GTK_WINDOW(m_window), 400, 300);
    gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);
    
    applyGrayTheme();
    setupUI();
}

MainWindow::~MainWindow() = default;

void MainWindow::applyGrayTheme() {
    GtkCssProvider* provider = gtk_css_provider_new();
    
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
            font-size: 16px;
            font-weight: bold;
            color: #cccccc;
            text-align: center;
            margin: 8px 0;
        }
        
        scale {
            margin: 15px 0;
        }
        
        scale trough {
            background: linear-gradient(to right, #555555 0%, #777777 50%, #555555 100%);
            border-radius: 4px;
            min-height: 6px;
        }
        
        scale slider {
            background-color: #ffffff;
            border: 2px solid #888888;
            border-radius: 50%;
            min-width: 16px;
            min-height: 16px;
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
            padding: 10px 20px;
            margin: 5px;
            font-weight: 500;
            min-height: 40px;
        }
        
        button:hover {
            background-color: #555555;
            border-color: #777777;
        }
        
        button:active {
            background-color: #3a3a3a;
        }
        
        .apply-button {
            background-color: #666666;
            font-weight: bold;
            min-width: 120px;
        }
        
        .apply-button:hover {
            background-color: #777777;
        }
        
        .install-button {
            background-color: #5a5a5a;
        }
        
        .install-button:hover {
            background-color: #6a6a6a;
        }
        
        .reset-button {
            background-color: #4a4a4a;
        }
        
        .reset-button:hover {
            background-color: #555555;
        }
        
        .button-box {
            margin-top: 20px;
            padding-top: 15px;
            border-top: 1px solid #555555;
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
    
    setupDisplayControls();
    
    // Button section
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_add_css_class(buttonBox, "button-box");
    gtk_widget_set_halign(buttonBox, GTK_ALIGN_CENTER);
    
    m_applyButton = gtk_button_new_with_label("Apply to All");
    gtk_widget_add_css_class(m_applyButton, "apply-button");
    g_signal_connect(m_applyButton, "clicked", G_CALLBACK(onApplyClicked), this);
    
    m_installButton = gtk_button_new_with_label("Set System Wide");
    gtk_widget_add_css_class(m_installButton, "install-button");
    g_signal_connect(m_installButton, "clicked", G_CALLBACK(onInstallClicked), this);
    
    m_resetButton = gtk_button_new_with_label("Reset All");
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
        
        std::string titleText = "Display: " + display.id;
        GtkWidget* titleLabel = gtk_label_new(titleText.c_str());
        gtk_widget_add_css_class(titleLabel, "display-title");
        gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
        
        GtkWidget* valueLabel = gtk_label_new("Normal (0)");
        gtk_widget_add_css_class(valueLabel, "value-label");
        m_valueLabels[display.id] = valueLabel;
        
        GtkWidget* vibranceScale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -100.0, 100.0, 1.0);
        gtk_scale_set_draw_value(GTK_SCALE(vibranceScale), FALSE);
        gtk_range_set_value(GTK_RANGE(vibranceScale), display.currentVibrance);
        
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), -100.0, GTK_POS_BOTTOM, "-100");
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), 0.0, GTK_POS_BOTTOM, "0");
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), 100.0, GTK_POS_BOTTOM, "+100");
        
        m_vibranceScales[display.id] = vibranceScale;
        g_object_set_data(G_OBJECT(vibranceScale), "display_id", g_strdup(display.id.c_str()));
        g_signal_connect(vibranceScale, "value-changed", G_CALLBACK(onVibranceChanged), this);
        
        gtk_box_append(GTK_BOX(displaySection), titleLabel);
        gtk_box_append(GTK_BOX(displaySection), valueLabel);
        gtk_box_append(GTK_BOX(displaySection), vibranceScale);
        
        gtk_box_append(GTK_BOX(m_mainBox), displaySection);
    }
}

void MainWindow::updateValueLabel(const std::string& displayId, int vibrance) {
    auto it = m_valueLabels.find(displayId);
    if (it != m_valueLabels.end()) {
        std::string text;
        if (vibrance == 0) {
            text = "Normal (0)";
        } else if (vibrance > 0) {
            text = "Enhanced (+" + std::to_string(vibrance) + ")";
        } else {
            text = "Reduced (" + std::to_string(vibrance) + ")";
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
        "Apply current vibrance settings to all displays?\n\nThis will make the changes permanent.",
        [window]() {
            window->applyAllSettings();
            window->showInfoDialog("✅ Settings applied successfully!");
        }
    );
}

void MainWindow::onInstallClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    if (window->m_controller->isSystemInstalled()) {
        window->showInfoDialog("✅ Vivid is already installed system-wide!");
        return;
    }
    
    window->showConfirmDialog(
        "Install Vivid system-wide?\n\nThis will make 'vivid' available from anywhere.",
        [window]() {
            if (window->m_controller->installSystemWide()) {
                window->showInfoDialog("✅ Vivid installed system-wide!\n\nYou can now run 'vivid' from anywhere.");
            } else {
                window->showInfoDialog("❌ Installation failed.\n\nMake sure you have admin privileges.");
            }
        }
    );
}

void MainWindow::onResetClicked(GtkButton* button __attribute__((unused)), gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    window->showConfirmDialog(
        "Reset all displays to normal vibrance?\n\nThis will set all values to 0.",
        [window]() {
            window->m_controller->resetAllDisplays();
            
            for (const auto& pair : window->m_vibranceScales) {
                gtk_range_set_value(GTK_RANGE(pair.second), 0.0);
                window->updateValueLabel(pair.first, 0);
            }
            
            window->showInfoDialog("✅ All displays reset to normal!");
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

// FIXED: Modern GTK4 dialogs without deprecated functions
void MainWindow::showConfirmDialog(const std::string& message, std::function<void()> callback) {
    GtkWidget* dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Confirm");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(m_window));
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 150);
    
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    
    GtkWidget* label = gtk_label_new(message.c_str());
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(buttonBox, GTK_ALIGN_CENTER);
    
    GtkWidget* cancelBtn = gtk_button_new_with_label("Cancel");
    GtkWidget* okBtn = gtk_button_new_with_label("OK");
    
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
    
    g_signal_connect(cancelBtn, "clicked", G_CALLBACK([](GtkButton*, gpointer data) {
        gtk_window_destroy(GTK_WINDOW(data));
    }), dialog);
    
    g_signal_connect(okBtn, "clicked", G_CALLBACK([](GtkButton*, gpointer data) {
        auto* callbackPtr = static_cast<std::function<void()>*>(g_object_get_data(G_OBJECT(data), "callback"));
        if (callbackPtr) (*callbackPtr)();
        gtk_window_destroy(GTK_WINDOW(data));
    }), dialog);
    
    gtk_window_present(GTK_WINDOW(dialog));
}

void MainWindow::showInfoDialog(const std::string& message) {
    GtkWidget* dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Information");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(m_window));
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 120);
    
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    
    GtkWidget* label = gtk_label_new(message.c_str());
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    
    GtkWidget* okBtn = gtk_button_new_with_label("OK");
    gtk_widget_set_halign(okBtn, GTK_ALIGN_CENTER);
    
    gtk_box_append(GTK_BOX(box), label);
    gtk_box_append(GTK_BOX(box), okBtn);
    
    gtk_window_set_child(GTK_WINDOW(dialog), box);
    
    g_signal_connect(okBtn, "clicked", G_CALLBACK([](GtkButton*, gpointer data) {
        gtk_window_destroy(GTK_WINDOW(data));
    }), dialog);
    
    gtk_window_present(GTK_WINDOW(dialog));
}

void MainWindow::show() {
    gtk_window_present(GTK_WINDOW(m_window));
}
