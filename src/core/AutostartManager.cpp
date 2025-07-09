#include "AutostartManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

AutostartManager::AutostartManager() 
    : m_minimizeToTray(false)
    , m_startWithProfiles(true)
    , m_delayedStart(3) {
    
    std::cout << "🚀 AutostartManager initialized" << std::endl;
}

AutostartManager::~AutostartManager() = default;

bool AutostartManager::isEnabled() {
    std::string filePath = getAutostartFilePath();
    bool exists = std::filesystem::exists(filePath);
    
    std::cout << "🔍 Checking autostart status:" << std::endl;
    std::cout << "  File path: " << filePath << std::endl;
    std::cout << "  Exists: " << (exists ? "Yes" : "No") << std::endl;
    
    if (exists) {
        // Validate the file is correct
        bool valid = validateDesktopFile();
        std::cout << "  Valid: " << (valid ? "Yes" : "No") << std::endl;
        return valid;
    }
    
    return false;
}

bool AutostartManager::enable() {
    std::cout << "🚀 Enabling autostart..." << std::endl;
    
    // Step 1: Create autostart directory
    if (!createAutostartDirectory()) {
        std::cerr << "❌ Failed to create autostart directory" << std::endl;
        return false;
    }
    
    // Step 2: Generate desktop file content
    std::string desktopContent = getDesktopFileContent();
    if (desktopContent.empty()) {
        std::cerr << "❌ Failed to generate desktop file content" << std::endl;
        return false;
    }
    
    // Step 3: Write desktop file
    if (!writeDesktopFile(desktopContent)) {
        std::cerr << "❌ Failed to write desktop file" << std::endl;
        return false;
    }
    
    // Step 4: Validate the written file
    if (!validateDesktopFile()) {
        std::cerr << "❌ Desktop file validation failed" << std::endl;
        removeDesktopFile(); // Clean up invalid file
        return false;
    }
    
    // Step 5: Test the autostart file
    if (!testAutostartFile()) {
        std::cerr << "⚠️ Autostart file test failed (but file was created)" << std::endl;
        // Don't return false here - the file might still work
    }
    
    std::cout << "✅ Autostart enabled successfully!" << std::endl;
    std::cout << "  File: " << getAutostartFilePath() << std::endl;
    
    return true;
}

bool AutostartManager::disable() {
    std::cout << "🛑 Disabling autostart..." << std::endl;
    
    std::string filePath = getAutostartFilePath();
    
    if (!std::filesystem::exists(filePath)) {
        std::cout << "ℹ️ Autostart file doesn't exist, nothing to disable" << std::endl;
        return true;
    }
    
    if (removeDesktopFile()) {
        std::cout << "✅ Autostart disabled successfully!" << std::endl;
        return true;
    } else {
        std::cerr << "❌ Failed to remove autostart file" << std::endl;
        return false;
    }
}

std::string AutostartManager::getStatus() {
    if (isEnabled()) {
        return "✅ Enabled - Vivid will start automatically on login";
    } else {
        return "❌ Disabled - Vivid will not start automatically";
    }
}

std::vector<std::string> AutostartManager::getDebugInfo() {
    return collectDebugInfo();
}

std::string AutostartManager::getAutostartDirectory() {
    const char* home = std::getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
    
    // Try XDG config directory first
    const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
    if (xdgConfig) {
        return std::string(xdgConfig) + "/autostart";
    }
    
    // Fallback to ~/.config/autostart
    return std::string(home) + "/.config/autostart";
}

std::string AutostartManager::getAutostartFilePath() {
    return getAutostartDirectory() + "/org.vivid.SaturationControl.desktop";
}

std::string AutostartManager::getExecutablePath() {
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::string(buffer);
    }
    
    // Fallback methods
    const char* argv0 = std::getenv("_");
    if (argv0) {
        return std::string(argv0);
    }
    
    return "vivid"; // Last resort
}

std::string AutostartManager::getDesktopFileContent() {
    std::string execPath = getExecutablePath();
    std::string execArgs = "";
    
    // Add startup options
    if (m_minimizeToTray) {
        execArgs += " --minimize";
    }
    
    if (m_startWithProfiles) {
        execArgs += " --apply-profiles";
    }
    
    if (m_delayedStart > 0) {
        execArgs += " --delay " + std::to_string(m_delayedStart);
    }
    
    // Generate desktop file content
    std::string content = R"([Desktop Entry]
Type=Application
Version=1.0
Name=Vivid Digital Vibrance Control
GenericName=Digital Vibrance Control
Comment=Automatically adjust screen vibrance and digital saturation
Exec=)" + execPath + execArgs + R"(
Icon=org.vivid.SaturationControl
Terminal=false
NoDisplay=false
Hidden=false
X-GNOME-Autostart-enabled=true
X-GNOME-Autostart-Delay=)" + std::to_string(m_delayedStart) + R"(
X-KDE-autostart-after=panel
X-KDE-StartupNotify=false
Categories=System;Settings;
Keywords=vibrance;saturation;color;display;autostart;
StartupNotify=false
StartupWMClass=vivid
)";

    std::cout << "📝 Generated desktop file content:" << std::endl;
    std::cout << content << std::endl;
    
    return content;
}

bool AutostartManager::createAutostartDirectory() {
    std::string dirPath = getAutostartDirectory();
    
    std::cout << "📁 Creating autostart directory: " << dirPath << std::endl;
    
    try {
        if (std::filesystem::exists(dirPath)) {
            std::cout << "  Directory already exists" << std::endl;
            return true;
        }
        
        if (std::filesystem::create_directories(dirPath)) {
            std::cout << "  ✅ Directory created successfully" << std::endl;
            
            // Set proper permissions (755)
            chmod(dirPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
            
            return true;
        } else {
            std::cerr << "  ❌ Failed to create directory" << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "  ❌ Exception creating directory: " << e.what() << std::endl;
        return false;
    }
}

bool AutostartManager::writeDesktopFile(const std::string& content) {
    std::string filePath = getAutostartFilePath();
    
    std::cout << "✍️ Writing desktop file: " << filePath << std::endl;
    
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "  ❌ Failed to open file for writing" << std::endl;
            return false;
        }
        
        file << content;
        file.close();
        
        if (file.fail()) {
            std::cerr << "  ❌ Failed to write file content" << std::endl;
            return false;
        }
        
        // Set proper permissions (644)
        chmod(filePath.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        
        std::cout << "  ✅ Desktop file written successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "  ❌ Exception writing file: " << e.what() << std::endl;
        return false;
    }
}

bool AutostartManager::removeDesktopFile() {
    std::string filePath = getAutostartFilePath();
    
    std::cout << "🗑️ Removing desktop file: " << filePath << std::endl;
    
    try {
        if (std::filesystem::remove(filePath)) {
            std::cout << "  ✅ File removed successfully" << std::endl;
            return true;
        } else {
            std::cerr << "  ❌ Failed to remove file (may not exist)" << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "  ❌ Exception removing file: " << e.what() << std::endl;
        return false;
    }
}

bool AutostartManager::validateDesktopFile() {
    std::string filePath = getAutostartFilePath();
    
    std::cout << "🔍 Validating desktop file..." << std::endl;
    
    if (!std::filesystem::exists(filePath)) {
        std::cout << "  ❌ File doesn't exist" << std::endl;
        return false;
    }
    
    // Check file permissions
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) == 0) {
        if (!(fileStat.st_mode & S_IRUSR)) {
            std::cout << "  ❌ File is not readable" << std::endl;
            return false;
        }
    }
    
    // Read and validate content
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "  ❌ Cannot open file for reading" << std::endl;
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    // Check required fields
    std::vector<std::string> requiredFields = {
        "[Desktop Entry]",
        "Type=Application",
        "Name=",
        "Exec="
    };
    
    for (const auto& field : requiredFields) {
        if (content.find(field) == std::string::npos) {
            std::cout << "  ❌ Missing required field: " << field << std::endl;
            return false;
        }
    }
    
    // Check if executable exists
    size_t execPos = content.find("Exec=");
    if (execPos != std::string::npos) {
        size_t lineEnd = content.find('\n', execPos);
        std::string execLine = content.substr(execPos + 5, lineEnd - execPos - 5);
        
        // Extract just the executable path (before first space)
        size_t spacePos = execLine.find(' ');
        std::string execPath = (spacePos != std::string::npos) ? 
                              execLine.substr(0, spacePos) : execLine;
        
        if (!std::filesystem::exists(execPath)) {
            std::cout << "  ⚠️ Executable doesn't exist: " << execPath << std::endl;
            // Don't return false - the path might be in PATH
        }
    }
    
    std::cout << "  ✅ Desktop file validation passed" << std::endl;
    return true;
}

bool AutostartManager::testAutostartFile() {
    std::cout << "🧪 Testing autostart file..." << std::endl;
    
    // Test with desktop-file-validate if available
    std::string filePath = getAutostartFilePath();
    std::string command = "desktop-file-validate \"" + filePath + "\" 2>&1";
    
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[256];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe)) {
            result += buffer;
        }
        int exitCode = pclose(pipe);
        
        if (exitCode == 0) {
            std::cout << "  ✅ Desktop file validation passed" << std::endl;
            return true;
        } else {
            std::cout << "  ⚠️ Desktop file validation warnings:" << std::endl;
            std::cout << "    " << result << std::endl;
            return false;
        }
    } else {
        std::cout << "  ℹ️ desktop-file-validate not available, skipping test" << std::endl;
        return true;
    }
}

std::vector<std::string> AutostartManager::collectDebugInfo() {
    std::vector<std::string> info;
    
    info.push_back("=== Vivid Autostart Debug Information ===");
    info.push_back("");
    
    // Basic info
    info.push_back("Status: " + getStatus());
    info.push_back("Autostart directory: " + getAutostartDirectory());
    info.push_back("Desktop file path: " + getAutostartFilePath());
    info.push_back("Executable path: " + getExecutablePath());
    info.push_back("");
    
    // Environment info
    info.push_back("Environment:");
    info.push_back("  HOME: " + std::string(std::getenv("HOME") ?: "not set"));
    info.push_back("  XDG_CONFIG_HOME: " + std::string(std::getenv("XDG_CONFIG_HOME") ?: "not set"));
    info.push_back("  XDG_CURRENT_DESKTOP: " + std::string(std::getenv("XDG_CURRENT_DESKTOP") ?: "not set"));
    info.push_back("  XDG_SESSION_TYPE: " + std::string(std::getenv("XDG_SESSION_TYPE") ?: "not set"));
    info.push_back("");
    
    // Directory info
    std::string autostartDir = getAutostartDirectory();
    info.push_back("Directory status:");
    info.push_back("  Exists: " + std::string(std::filesystem::exists(autostartDir) ? "Yes" : "No"));
    
    if (std::filesystem::exists(autostartDir)) {
        try {
            auto perms = std::filesystem::status(autostartDir).permissions();
            info.push_back("  Permissions: " + std::to_string(static_cast<int>(perms)));
        } catch (...) {
            info.push_back("  Permissions: Unable to check");
        }
    }
    info.push_back("");
    
    // File info
    std::string filePath = getAutostartFilePath();
    info.push_back("Desktop file status:");
    info.push_back("  Exists: " + std::string(std::filesystem::exists(filePath) ? "Yes" : "No"));
    
    if (std::filesystem::exists(filePath)) {
        try {
            auto size = std::filesystem::file_size(filePath);
            info.push_back("  Size: " + std::to_string(size) + " bytes");
            
            auto perms = std::filesystem::status(filePath).permissions();
            info.push_back("  Permissions: " + std::to_string(static_cast<int>(perms)));
            
            // Read first few lines
            std::ifstream file(filePath);
            if (file.is_open()) {
                std::string line;
                int lineCount = 0;
                info.push_back("  Content preview:");
                while (std::getline(file, line) && lineCount < 5) {
                    info.push_back("    " + line);
                    lineCount++;
                }
                if (lineCount == 5) {
                    info.push_back("    ...");
                }
                file.close();
            }
        } catch (const std::exception& e) {
            info.push_back("  Error reading file: " + std::string(e.what()));
        }
    }
    info.push_back("");
    
    // System info
    info.push_back("System autostart support:");
    
    // Check for common autostart directories
    std::vector<std::string> systemDirs = {
        "/etc/xdg/autostart",
        "/usr/share/autostart",
        "/usr/local/share/autostart"
    };
    
    for (const auto& dir : systemDirs) {
        info.push_back("  " + dir + ": " + 
                      (std::filesystem::exists(dir) ? "Exists" : "Not found"));
    }
    
    return info;
}
