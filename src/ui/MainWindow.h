#pragma once
#include <gtk/gtk.h>
#include <memory>
#include <map>
#include "../core/VibranceController.h"

class MainWindow {
public:
    MainWindow(GtkApplication* app);
    ~MainWindow();
    
    void show();

private:
    GtkWidget* m_window = nullptr;
    GtkWidget* m_notebook = nullptr;
    GtkWidget* m_focusCheckbox = nullptr;
    GtkWidget* m_programListBox = nullptr;
    GtkWidget* m_addProgramButton = nullptr;
    GtkWidget* m_removeProgramButton = nullptr;
    
    // Display controls
    std::map<std::string, GtkWidget*> m_vibranceScales;
    std::map<std::string, GtkWidget*> m_vibranceSpins;
    std::map<std::string, GtkWidget*> m_valueLabels;
    
    std::unique_ptr<VibranceController> m_controller;
    
    // UI setup
    void setupUI();
    void setupHeaderBar();
    void setupDisplayTabs();
    void setupDisplayTab(const Display& display);
    void setupProgramSection();
    void updateProgramList();
    
    // Auto-optimization
    void autoApplySettings();
    void updateDisplayControls(const std::string& displayId, int vibrance);
    void updateValueLabel(GtkWidget* label, int vibrance);
    
    // Event handlers - ALL WORKING
    static void onVibranceChanged(GtkRange* range, gpointer user_data);
    static void onVibranceSpinChanged(GtkSpinButton* spin, gpointer user_data);
    static void onPresetClicked(GtkButton* button, gpointer user_data);
    static void onAutoOptimizeClicked(GtkButton* button, gpointer user_data);
    static void onHelpClicked(GtkButton* button, gpointer user_data);
    static void onFileMenuClicked(GtkButton* button, gpointer user_data);
    static void onFocusToggled(GtkCheckButton* button, gpointer user_data);
    static void onAddProgram(GtkButton* button, gpointer user_data);
    static void onRemoveProgram(GtkButton* button, gpointer user_data);
    static void onProgramDoubleClick(GtkListBox* listbox, GtkListBoxRow* row, gpointer user_data);
    
    void showProgramDialog(const std::string& programPath = "");
    void applyUnifiedTheme();
};
