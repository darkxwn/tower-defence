#include "SettingsManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

#ifdef ANDROID
#include <SFML/System/NativeActivity.hpp>
#include <android/native_activity.h>
#endif

// Инициализация менеджера настроек
SettingsManager::SettingsManager() {
    settingsPath = getSavePath();
    setDefaults();
    load();
}

// Получение пути сохранения
std::string SettingsManager::getSavePath() {
#ifdef ANDROID
    // внутренняя память приложения для android
    ANativeActivity* activity = sf::getNativeActivity();
    return std::string(activity->internalDataPath) + "/settings.cfg";
#else
    // папка с игрой для ПК
    return "data/config/settings.cfg";
#endif
}

// Установка настроек по умолчанию
void SettingsManager::setDefaults() {
    settings["music_volume"] = "100";
    settings["sfx_volume"] = "100";
#ifdef ANDROID
    settings["ui_scale"] = "1.0";
    settings["sensitivity"] = "1.45";
#else
    settings["ui_scale"] = "1.0";
    settings["sensitivity"] = "1.0";
#endif  
    settings["fullscreen"] = "0";
}

// Загрузка настроек из файла
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

// Сохранение настроек в файл
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

// Получение значения для типа int
template<>
int SettingsManager::get<int>(const std::string& key) const {
    if (settings.find(key) == settings.end()) return 0;
    try { 
        return std::stoi(settings.at(key)); 
    } catch (...) { 
        return 0; 
    }
}

// Получение значения для типа float
template<>
float SettingsManager::get<float>(const std::string& key) const {
    if (settings.find(key) == settings.end()) return 0.0f;
    try { return std::stof(settings.at(key)); } catch (...) { return 0.0f; }
}

// Получение значения для типа bool
template<>
bool SettingsManager::get<bool>(const std::string& key) const {
    if (settings.find(key) == settings.end()) return false;
    const std::string& val = settings.at(key);
    return (val == "true" || val == "1");
}

// Получение значения для типа string
template<>
std::string SettingsManager::get<std::string>(const std::string& key) const {
    if (settings.find(key) == settings.end()) return "";
    return settings.at(key);
}

// Получение значения с возвратом значения по умолчанию для типа int
template<>
int SettingsManager::get<int>(const std::string& key, const int& defaultValue) const {
    if (settings.find(key) == settings.end()) return defaultValue;
    try { return std::stoi(settings.at(key)); } catch (...) { return defaultValue; }
}

// Получение значения с возвратом значения по умолчанию для типа float
template<>
float SettingsManager::get<float>(const std::string& key, const float& defaultValue) const {
    if (settings.find(key) == settings.end()) return defaultValue;
    try { return std::stof(settings.at(key)); } catch (...) { return defaultValue; }
}

// Получение значения с возвратом значения по умолчанию для типа bool
template<>
bool SettingsManager::get<bool>(const std::string& key, const bool& defaultValue) const {
    if (settings.find(key) == settings.end()) return defaultValue;
    const std::string& val = settings.at(key);
    return (val == "true" || val == "1");
}

// Получение значения с возвратом значения по умолчанию для типа string
template<>
std::string SettingsManager::get<std::string>(const std::string& key, const std::string& defaultValue) const {
    if (settings.find(key) == settings.end()) return defaultValue;
    return settings.at(key);
}

// Изменение значения для типа int
template<>
void SettingsManager::set<int>(const std::string& key, const int& value) {
    settings[key] = std::to_string(value);
}

// Изменение значения для типа float
template<>
void SettingsManager::set<float>(const std::string& key, const float& value) {
    settings[key] = std::to_string(value);
}

// Изменение значения для типа bool
template<>
void SettingsManager::set<bool>(const std::string& key, const bool& value) {
    settings[key] = value ? "1" : "0";
}

// Изменение значения для типа string
template<>
void SettingsManager::set<std::string>(const std::string& key, const std::string& value) {
    settings[key] = value;
}
