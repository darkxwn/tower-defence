#pragma once
#include <string>
#include <map>

class SettingsManager {
private:
    // Храним настройки как ключ-значение (string-string), чтобы легко расширять
    std::map<std::string, std::string> settings;
    std::string settingsPath;

    // Приватные методы для парсинга
    void setDefaults();
    std::string getSavePath(); // Самый важный метод для кроссплатформа

public:
    SettingsManager();

    void load();
    void save();

    // Геттеры с приведением типов
    float getFloat(const std::string& key);
    int   getInt(const std::string& key);
    bool  getBool(const std::string& key);
    std::string getString(const std::string& key, const std::string& defaultVal);

    // Сеттеры
    void set(const std::string& key, const std::string& value);
    void set(const std::string& key, float value);
    void set(const std::string& key, int value);
    void set(const std::string& key, bool value);
};