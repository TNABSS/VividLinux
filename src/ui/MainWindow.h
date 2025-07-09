#pragma once

#include <gtk/gtk.h>
#include <memory>
#include "../core/VividManager.h"

class MainWindow {
public:
    MainWindow(GtkApplication* app, VividManager* manager);
    ~MainWindow();
    
    void show();

private:
    GtkWidget* m_window = nullptr;
    GtkWidget* m_notebook = nullptr;
    GtkWidget* m_focusCheckbox = nullptr;
    GtkWidget* m_statusLabel = nullptr;
    GtkWidget* m_programList = nullptr;
    GtkWidget* m_addProgramButton = nullptr;
    GtkWidget* m_removeProgramButton = nullptr;
    
    // Display tabs and controls
    std::map<std::string, GtkWidget*> m_displayTabs;
    std::map<std::string, GtkWidget*> m_vibranceScales;
    std::map<std::string, GtkWidget*> m_vibranceSpins;
    
    VividManager* m_manager;
    std::string m_currentDisplay;
    
    // GTK callbacks
    static void onDisplayTabChanged(GtkNotebook* notebook, GtkWidget* page, guint page_num, gpointer user_data);
    static void onVibranceChanged(GtkRange* range, gpointer user_data);
    static void onVibranceSpinChanged(GtkSpinButton* spin, gpointer user_data);
    static void onFocusCheckboxToggled(GtkCheckButton* button, gpointer user_data);
    static void onAddProgramClicked(GtkButton* button, gpointer user_data);
    static void onRemoveProgramClicked(GtkButton* button, gpointer user_data);
    static void onProgramListSelectionChanged(GtkTreeSelection* selection, gpointer user_data);
    
    void setupUI();
    void setupMenuBar();
    void setupDisplayTabs();
    void setupProgramList();
    void updateVibranceControls();
    void updateProgramList();
    void showAddProgramDialog();
    void applyDarkTheme();
};
