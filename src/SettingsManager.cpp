#include "SettingsManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

#ifdef ANDROID
#include <SFML/System/NativeActivity.hpp>
#include <android/native_activity.h>
#endif

// Конструктор менеджера настроек
SettingsManager::SettingsManager() {
    settingsPath = getSavePath();
    setDefaults();
    load();
}

// Получение пути сохранения
std::string SettingsManager::getSavePath() {
#ifdef ANDROID
    // на android пишем во внутреннюю память приложения
    ANativeActivity* activity = sf::getNativeActivity();
    return std::string(activity->internalDataPath) + "/settings.cfg";
#else
    // на ПК пишем в папку с игрой
    return "data/config/settings.cfg";
#endif
}

// Установка настроек по умолчанию
void SettingsManager::setDefaults() {
    settings["music_volume"] = "100";
    settings["sfx_volume"] = "100";
#ifdef ANDROID
    settings["ui_scale"] = "2.0";
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

// Получение целого значения
int SettingsManager::getInt(const std::string& key) {
    return std::stoi(settings[key]);
}

// Получение числового значения
float SettingsManager::getFloat(const std::string& key) {
    return std::stof(settings[key]);
}

// Получение логического значения
bool SettingsManager::getBool(const std::string& key) {
    return (settings[key] == "true" || settings[key] == "1");
}

// Изменение строкового значения
void SettingsManager::set(const std::string& key, const std::string& value) {
    settings[key] = value;
}

// Изменение числового значения
void SettingsManager::set(const std::string& key, float value) {
    settings[key] = std::to_string(value);
}

// Изменение целого значения
void SettingsManager::set(const std::string& key, int value) {
    settings[key] = std::to_string(value);
}

// Изменение логического значения
void SettingsManager::set(const std::string& key, bool value) {
    settings[key] = std::to_string(value);
}