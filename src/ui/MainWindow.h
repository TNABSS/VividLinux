#pragma once
#include <gtk/gtk.h>
#include <memory>
#include <map>
#include <functional>
#include "../core/VibranceController.h"

class MainWindow {
public:
    MainWindow(GtkApplication* app);
    ~MainWindow();
    void show();

private:
    GtkWidget* m_window = nullptr;
    GtkWidget* m_mainBox = nullptr;
    std::map<std::string, GtkWidget*> m_vibranceScales;
    std::map<std::string, GtkWidget*> m_valueLabels;
    std::unique_ptr<VibranceController> m_controller;
    
    void setupUI();
    void setupDisplayControls();
    void applyTheme();
    void updateValueLabel(const std::string& displayId, int vibrance);
    
    static void onVibranceChanged(GtkRange* range, gpointer user_data);
    static void onResetClicked(GtkButton* button, gpointer user_data);
    static void onInstallClicked(GtkButton* button, gpointer user_data);
};
