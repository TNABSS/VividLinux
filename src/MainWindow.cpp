#include "MainWindow.h"
#include <iostream>
#include <iomanip>
#include <sstream>

MainWindow::MainWindow(GtkApplication* app) {
    controller = std::make_unique<SaturationController>();
    
    // Create main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Vivid - Saturation Control");
    gtk_window_set_default_size(GTK_WINDOW(window), 450, 300);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    
    setupUI();
    updateDisplayList();
    updateStatus();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    // Create main vertical box
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    
    // Title
    GtkWidget* titleLabel = gtk_label_new(nullptr);
    gtk_label_set_markup(GTK_LABEL(titleLabel), "<span size='large' weight='bold'>Vivid - Saturation Control</span>");
    gtk_widget_set_halign(titleLabel, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(vbox), titleLabel);
    
    // Status section
    GtkWidget* statusFrame = gtk_frame_new("Status");
    statusLabel = gtk_label_new("Initializing...");
    gtk_widget_set_margin_start(statusLabel, 10);
    gtk_widget_set_margin_end(statusLabel, 10);
    gtk_widget_set_margin_top(statusLabel, 10);
    gtk_widget_set_margin_bottom(statusLabel, 10);
    gtk_frame_set_child(GTK_FRAME(statusFrame), statusLabel);
    gtk_box_append(GTK_BOX(vbox), statusFrame);
    
    // Display selection
    GtkWidget* displayLabel = gtk_label_new("Select Display:");
    gtk_widget_set_halign(displayLabel, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), displayLabel);
    
    displayCombo = gtk_combo_box_text_new();
    g_signal_connect(displayCombo, "changed", G_CALLBACK(onDisplayChanged), this);
    gtk_box_append(GTK_BOX(vbox), displayCombo);
    
    // Saturation control
    GtkWidget* saturationLabel = gtk_label_new("Saturation:");
    gtk_widget_set_halign(saturationLabel, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), saturationLabel);
    
    // Percentage display
    percentLabel = gtk_label_new("100%");
    gtk_widget_set_halign(percentLabel, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(vbox), percentLabel);
    
    saturationScale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 200.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(saturationScale), GTK_POS_BOTTOM);
    gtk_scale_set_digits(GTK_SCALE(saturationScale), 0);
    gtk_range_set_value(GTK_RANGE(saturationScale), 100.0);
    
    // Add marks for common values
    gtk_scale_add_mark(GTK_SCALE(saturationScale), 0.0, GTK_POS_BOTTOM, "0%");
    gtk_scale_add_mark(GTK_SCALE(saturationScale), 50.0, GTK_POS_BOTTOM, "50%");
    gtk_scale_add_mark(GTK_SCALE(saturationScale), 100.0, GTK_POS_BOTTOM, "100%");
    gtk_scale_add_mark(GTK_SCALE(saturationScale), 150.0, GTK_POS_BOTTOM, "150%");
    gtk_scale_add_mark(GTK_SCALE(saturationScale), 200.0, GTK_POS_BOTTOM, "200%");
    
    g_signal_connect(saturationScale, "value-changed", G_CALLBACK(onSaturationChanged), this);
    gtk_box_append(GTK_BOX(vbox), saturationScale);
    
    // Help text
    GtkWidget* helpLabel = gtk_label_new("0% = Grayscale, 100% = Normal, 200% = Oversaturated");
    gtk_widget_set_halign(helpLabel, GTK_ALIGN_CENTER);
    gtk_label_set_markup(GTK_LABEL(helpLabel), "<span size='small' style='italic'>0% = Grayscale, 100% = Normal, 200% = Oversaturated</span>");
    gtk_box_append(GTK_BOX(vbox), helpLabel);
    
    // Reset button
    resetButton = gtk_button_new_with_label("Reset to Normal (100%)");
    g_signal_connect(resetButton, "clicked", G_CALLBACK(onResetClicked), this);
    gtk_box_append(GTK_BOX(vbox), resetButton);
    
    gtk_window_set_child(GTK_WINDOW(window), vbox);
}

void MainWindow::updateDisplayList() {
    // Clear existing items
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(displayCombo));
    
    auto displays = controller->getDisplays();
    for (const auto& display : displays) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(displayCombo), display.c_str());
    }
    
    if (!displays.empty()) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(displayCombo), 0);
        currentDisplay = displays[0];
        updateSaturationSlider();
    }
}

void MainWindow::updateStatus() {
    std::string status = "Method: " + controller->getCurrentMethod();
    if (!controller->isInitialized()) {
        status += " (Demo Mode - No actual changes will be made)";
    }
    gtk_label_set_text(GTK_LABEL(statusLabel), status.c_str());
}

void MainWindow::updatePercentLabel() {
    double value = gtk_range_get_value(GTK_RANGE(saturationScale));
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0) << value << "%";
    gtk_label_set_text(GTK_LABEL(percentLabel), oss.str().c_str());
}

void MainWindow::onDisplayChanged(GtkComboBox* combo, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    
    gchar* text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
    if (text) {
        window->currentDisplay = text;
        window->updateSaturationSlider();
        std::cout << "Selected display: " << window->currentDisplay << "\n";
        g_free(text);
    }
}

void MainWindow::onSaturationChanged(GtkRange* range, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    window->updatePercentLabel();
    
    if (window->currentDisplay.empty()) return;
    
    double value = gtk_range_get_value(range);
    float saturation = static_cast<float>(value / 100.0); // Convert percentage to 0-2 range
    
    window->controller->setSaturation(window->currentDisplay, saturation);
}

void MainWindow::onResetClicked(GtkButton* button, gpointer user_data) {
    auto* window = static_cast<MainWindow*>(user_data);
    if (window->currentDisplay.empty()) return;
    
    window->controller->resetSaturation(window->currentDisplay);
    gtk_range_set_value(GTK_RANGE(window->saturationScale), 100.0);
    window->updatePercentLabel();
}

void MainWindow::updateSaturationSlider() {
    if (currentDisplay.empty()) return;
    
    float saturation = controller->getSaturation(currentDisplay);
    gtk_range_set_value(GTK_RANGE(saturationScale), saturation * 100.0);
    updatePercentLabel();
}

void MainWindow::show() {
    gtk_window_present(GTK_WINDOW(window));
}
