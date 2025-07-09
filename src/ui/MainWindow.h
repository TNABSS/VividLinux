#pragma once
#include <gtk/gtk.h>
#include <memory>
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
    
    std::unique_ptr<VibranceController> m_controller;
    
    // UI setup
    void setupUI();
    void setupMenuBar();
    void setupDisplayTabs();
    void setupDisplayTab(const Display& display);
    void setupProgramSection();
    void updateProgramList();
    
    // Event handlers
    static void onVibranceChanged(GtkRange* range, gpointer user_data);
    static void onVibranceSpinChanged(GtkSpinButton* spin, gpointer user_data);
    static void onFocusToggled(GtkCheckButton* button, gpointer user_data);
    static void onAddProgram(GtkButton* button, gpointer user_data);
    static void onRemoveProgram(GtkButton* button, gpointer user_data);
    static void onProgramDoubleClick(GtkListBox* listbox, GtkListBoxRow* row, gpointer user_data);
    
    void showProgramDialog(const std::string& programPath = "");
    void applyDarkTheme();
};
