#include "SettingsManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

#ifdef ANDROID
#include <SFML/System/NativeActivity.hpp>
#include <android/native_activity.h>
#endif

SettingsManager::SettingsManager() {
    settingsPath = getSavePath();
    setDefaults();
    load();
}

std::string SettingsManager::getSavePath() {
#ifdef ANDROID
    // На Android пишем во внутреннюю память приложения (sandbox)
    // Обычно это /data/data/com.package.name/files/
    ANativeActivity* activity = sf::getNativeActivity();
    return std::string(activity->internalDataPath) + "/settings.cfg";
#else
    // На ПК для простоты пишем в папку с игрой
    return "data/config/settings.cfg";
#endif
}

void SettingsManager::setDefaults() {
    settings["music_volume"] = "100";
    settings["sfx_volume"] = "100";
#ifdef ANDROID
    settings["ui_scale"] = "2.0";
    settings["sensivity"] = "1.45";
#else
    settings["ui_scale"] = "1.0";
    settings["sensivity"] = "1.0";
#endif  
    settings["fullscreen"] = "0";
}

void SettingsManager::load() {
    std::ifstream file(settingsPath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        size_t sep = line.find('=');
        if (sep != std::string::npos) {
            std::string key = line.substr(0, sep);
            std::string val = line.substr(sep + 1);
            settings[key] = val;
        }
    }
}

void SettingsManager::save() {
    std::ofstream file(settingsPath);
    if (!file.is_open()) {
        std::cerr << "Failed to save settings to " << settingsPath << std::endl;
        return;
    }

    for (auto const& [key, val] : settings) {
        file << key << "=" << val << "\n";
    }
}

// Хелперы для типов
int SettingsManager::getInt(const std::string& key) {
    //if (settings.find(key) == settings.end()) return defaultVal;
    return std::stoi(settings[key]);
}

float SettingsManager::getFloat(const std::string& key) {
    //if (settings.find(key) == settings.end()) return defaultVal;
    return std::stof(settings[key]);
}

bool SettingsManager::getBool(const std::string& key) {
    //if (settings.find(key) == settings.end()) return defaultVal;
    return (settings[key] == "true" || settings[key] == "1");
}

void SettingsManager::set(const std::string& key, const std::string& value) {
    settings[key] = value;
}

void SettingsManager::set(const std::string& key, float value) {
    settings[key] = std::to_string(value);
}

void SettingsManager::set(const std::string& key, int value) {
    settings[key] = std::to_string(value);
}

void SettingsManager::set(const std::string& key, bool value) {
    settings[key] = std::to_string(value);
}