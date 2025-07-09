#pragma once
#include <gtk/gtk.h>
#include <memory>
#include <map>
#include <functional>  // FIXED: Added missing include
#include "../core/VibranceController.h"

class MainWindow {
public:
    MainWindow(GtkApplication* app);
    ~MainWindow();
    
    void show();

private:
    GtkWidget* m_window = nullptr;
    GtkWidget* m_mainBox = nullptr;
    GtkWidget* m_applyButton = nullptr;
    GtkWidget* m_installButton = nullptr;
    GtkWidget* m_resetButton = nullptr;
    
    // Display controls
    std::map<std::string, GtkWidget*> m_vibranceScales;
    std::map<std::string, GtkWidget*> m_valueLabels;
    
    std::unique_ptr<VibranceController> m_controller;
    
    // UI setup
    void setupUI();
    void setupDisplayControls();
    void applyGrayTheme();
    
    // Event handlers
    static void onVibranceChanged(GtkRange* range, gpointer user_data);
    static void onApplyClicked(GtkButton* button, gpointer user_data);
    static void onInstallClicked(GtkButton* button, gpointer user_data);
    static void onResetClicked(GtkButton* button, gpointer user_data);
    
    // Helpers
    void updateValueLabel(const std::string& displayId, int vibrance);
    void showConfirmDialog(const std::string& message, std::function<void()> callback);  // FIXED: Now properly declared
    void showInfoDialog(const std::string& message);
    void applyAllSettings();
};
