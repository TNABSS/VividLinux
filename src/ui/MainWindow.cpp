#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow(GtkApplication* app) {
    m_controller = std::make_unique<VibranceController>();
    
    m_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(m_window), "Vivid");
    gtk_window_set_default_size(GTK_WINDOW(m_window), 400, 250);
    gtk_window_set_resizable(GTK_WINDOW(m_window), FALSE);
    
    applyTheme();
    setupUI();
}

MainWindow::~MainWindow() = default;

void MainWindow::applyTheme() {
    GtkCssProvider* provider = gtk_css_provider_new();
    
    const char* css = R"(
        window {
            background-color: #2e2e2e;
            color: #ffffff;
        }
        
        .main-container {
            background-color: #2e2e2e;
            padding: 20px;
        }
        
        .display-section {
            background-color: #3a3a3a;
            border: 1px solid #555555;
            border-radius: 6px;
            padding: 15px;
            margin: 8px 0;
        }
        
        .display-title {
            font-size: 14px;
            font-weight: bold;
            color: #ffffff;
            margin-bottom: 8px;
        }
        
        .value-label {
            font-size: 16px;
            font-weight: bold;
            color: #cccccc;
            margin: 6px 0;
        }
        
        scale {
            margin: 10px 0;
        }
        
        scale trough {
            background-color: #555555;
            border-radius: 3px;
            min-height: 6px;
        }
        
        scale slider {
            background-color: #ffffff;
            border: 1px solid #888888;
            border-radius: 50%;
            min-width: 16px;
            min-height: 16px;
        }
        
        button {
            background-color: #4a4a4a;
            color: #ffffff;
            border: 1px solid #666666;
            border-radius: 4px;
            padding: 8px 16px;
            margin: 4px;
            min-height: 32px;
        }
        
        button:hover {
            background-color: #555555;
        }
        
        .button-box {
            margin-top: 15px;
            padding-top: 10px;
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
    
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_add_css_class(buttonBox, "button-box");
    gtk_widget_set_halign(buttonBox, GTK_ALIGN_CENTER);
    
    GtkWidget* resetButton = gtk_button_new_with_label("Reset");
    g_signal_connect(resetButton, "clicked", G_CALLBACK(onResetClicked), this);
    
    GtkWidget* installButton = gtk_button_new_with_label("Install");
    g_signal_connect(installButton, "clicked", G_CALLBACK(onInstallClicked), this);
    
    gtk_box_append(GTK_BOX(buttonBox), resetButton);
    gtk_box_append(GTK_BOX(buttonBox), installButton);
    gtk_box_append(GTK_BOX(m_mainBox), buttonBox);
}

void MainWindow::setupDisplayControls() {
    auto displays = m_controller->getDisplays();
    
    for (const auto& display : displays) {
        GtkWidget* displaySection = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_widget_add_css_class(displaySection, "display-section");
        
        std::string titleText = "Display: " + display.id;
        GtkWidget* titleLabel = gtk_label_new(titleText.c_str());
        gtk_widget_add_css_class(titleLabel, "display-title");
        gtk_widget_set_halign(titleLabel, GTK_ALIGN_START);
        
        GtkWidget* valueLabel = gtk_label_new("0");
        gtk_widget_add_css_class(valueLabel, "value-label");
        gtk_widget_set_halign(valueLabel, GTK_ALIGN_CENTER);
        m_valueLabels[display.id] = valueLabel;
        
        GtkWidget* vibranceScale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -100.0, 100.0, 1.0);
        gtk_scale_set_draw_value(GTK_SCALE(vibranceScale), FALSE);
        gtk_range_set_value(GTK_RANGE(vibranceScale), display.currentVibrance);
        
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), -100.0, GTK_POS_BOTTOM, "-100");
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), 0.0, GTK_POS_BOTTOM, "0");
        gtk_scale_add_mark(GTK_SCALE(vibranceScale), 100.0, GTK_POS_BOTTOM, "100");
        
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
        gtk_label_set_text(GTK_LABEL(it->second), std::to_string(vibrance).c_str());
    }
}

void MainWindow::onVibranceChanged(GtkRange* range, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    const char* displayId = static_cast<const char*>(g_object_get_data(G_OBJECT(range), "display_id"));
    if (!displayId) return;
    
    double value = gtk_range_get_value(range);
    int vibrance = static_cast<int>(value);
    
    window->m_controller->setVibrance(displayId, vibrance);
    window->updateValueLabel(displayId, vibrance);
}

void MainWindow::onResetClicked(GtkButton* button, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    window->m_controller->resetAllDisplays();
    
    for (const auto& pair : window->m_vibranceScales) {
        gtk_range_set_value(GTK_RANGE(pair.second), 0.0);
        window->updateValueLabel(pair.first, 0);
    }
}

void MainWindow::onInstallClicked(GtkButton* button, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    window->m_controller->installSystemWide();
}

void MainWindow::show() {
    gtk_window_present(GTK_WINDOW(m_window));
}
