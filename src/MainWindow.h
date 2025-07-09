#pragma once
#include <gtk/gtk.h>
#include <memory>
#include "SaturationController.h"

// Simple GTK4 main window
class MainWindow {
public:
    MainWindow(GtkApplication* app);
    ~MainWindow();
    
    void show();

private:
    GtkWidget* window;
    GtkWidget* displayCombo;
    GtkWidget* saturationScale;
    GtkWidget* statusLabel;
    GtkWidget* resetButton;
    GtkWidget* percentLabel;
    
    std::unique_ptr<SaturationController> controller;
    std::string currentDisplay;
    
    // GTK callbacks
    static void onDisplayChanged(GtkComboBox* combo, gpointer user_data);
    static void onSaturationChanged(GtkRange* range, gpointer user_data);
    static void onResetClicked(GtkButton* button, gpointer user_data);
    
    void setupUI();
    void updateDisplayList();
    void updateSaturationSlider();
    void updateStatus();
    void updatePercentLabel();
};
