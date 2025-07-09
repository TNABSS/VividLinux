#pragma once

#include <gtk/gtk.h>
#include <memory>
#include <filesystem>
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
    GtkWidget* m_programListBox = nullptr;
    GtkWidget* m_addProgramButton = nullptr;
    GtkWidget* m_removeProgramButton = nullptr;
    
    // Display tabs and controls
    std::map<std::string, GtkWidget*> m_displayTabs;
    std::map<std::string, GtkWidget*> m_vibranceScales;
    std::map<std::string, GtkWidget*> m_vibranceLabels;
    
    VividManager* m_manager;
    std::string m_currentDisplay;

    GtkWidget* m_autostartCheckbox = nullptr;
    GtkWidget* m_autostartStatusLabel = nullptr;
    
    // GTK callbacks
    static void onDisplayTabChanged(GtkNotebook* notebook, GtkWidget* page, guint page_num, gpointer user_data);
    static void onVibranceChanged(GtkRange* range, gpointer user_data);
    static void onPresetClicked(GtkButton* button, gpointer user_data);
    static void onResetAllClicked(GtkButton* button, gpointer user_data);
    static void onFocusCheckboxToggled(GtkCheckButton* button, gpointer user_data);
    static void onAddProgramClicked(GtkButton* button, gpointer user_data);
    static void onRemoveProgramClicked(GtkButton* button, gpointer user_data);
    static void onSaveProgramProfile(GtkButton* button, gpointer user_data);

    static void onAutostartToggled(GtkCheckButton* button, gpointer user_data);
    static void onAutostartDebugClicked(GtkButton* button, gpointer user_data);
    
    void setupUI();
    void setupModernHeader();
    void setupSafetyWarning(GtkWidget* parent);
    void setupFocusCard(GtkWidget* parent);
    void setupDisplayTabs();
    void setupDisplayTab(const VividDisplay& display);
    void setupProgramCard(GtkWidget* parent);
    void setupProgramList();
    void setupStatusBar(GtkWidget* parent);
    void setupAutostartCard(GtkWidget* parent);
    
    void showFileBrowserDialog();
    void showProgramConfigDialog(const std::string& programPath);
    void showAutostartDebugDialog();
    
    void updateVibranceControls();
    void updateProgramList();
    void updateAutostartStatus();
    void applyModernTheme();
};
