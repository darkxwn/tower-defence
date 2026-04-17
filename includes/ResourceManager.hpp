#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС RESOURCEMANAGER
//
///////////////////////////////////////////////////////////////////////////

class ResourceManager {
private:
    static std::map<std::string, sf::Texture> textures; // хранилище текстур
    static std::map<std::string, sf::Font> fonts; // хранилище шрифтов

public:
    // Получение текстуры по имени
    static sf::Texture& get(const std::string& name);

    // Загрузка текстуры
    static void load(const std::string& name, const std::string& path);

    // Получение шрифта по имени
    static sf::Font& getFont(const std::string& name);

    // Загрузка шрифта
    static void loadFont(const std::string& name, const std::string& path);
};
