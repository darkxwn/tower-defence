#pragma once
#include <string>
#include <map>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС SETTINGSMANAGER
//
///////////////////////////////////////////////////////////////////////////

class SettingsManager {
private:
    std::map<std::string, std::string> settings; // хранилище настроек
    std::string settingsPath; // путь к файлу настроек

    // Установка настроек по умолчанию
    void setDefaults();

    // Получение пути сохранения
    std::string getSavePath();

public:
    // Инициализация менеджера
    SettingsManager();

    // Загрузка настроек из файла
    void load();

    // Сохранение настроек в файл
    void save();

    // Получение значения настройки по ключу
    template<typename T>
    T get(const std::string& key) const;

    // Получение значения настройки с возвратом значения по умолчанию при отсутствии ключа
    template<typename T>
    T get(const std::string& key, const T& defaultValue) const;

    // Изменение значения настройки
    template<typename T>
    void set(const std::string& key, const T& value);
};
