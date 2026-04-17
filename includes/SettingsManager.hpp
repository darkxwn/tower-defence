#pragma once
#include <string>
#include <map>

class SettingsManager {
private:
    std::map<std::string, std::string> settings; // хранилище настроек
    std::string settingsPath; // путь к файлу настроек

    // Установка настроек по умолчанию
    void setDefaults();

    // Получение пути сохранения
    std::string getSavePath();

public:
    SettingsManager();

    void load();
    void save();

    // Получение числового значения
    float getFloat(const std::string& key);

    // Получение целого значения
    int   getInt(const std::string& key);

    // Получение логического значения
    bool  getBool(const std::string& key);

    // Получение строкового значения
    std::string getString(const std::string& key, const std::string& defaultVal);

    // Изменение строкового значения
    void set(const std::string& key, const std::string& value);

    // Изменение числового значения
    void set(const std::string& key, float value);

    // Изменение целого значения
    void set(const std::string& key, int value);

    // Изменение логического значения
    void set(const std::string& key, bool value);
};