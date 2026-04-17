#include "ResourceManager.hpp"

std::map<std::string, sf::Texture> ResourceManager::textures;
std::map<std::string, sf::Font> ResourceManager::fonts;

// Получение текстуры по имени
sf::Texture& ResourceManager::get(const std::string& name) {
    auto it = textures.find(name);
    if (it == textures.end())
        throw std::runtime_error("[ERROR]: Текстура не найдена: " + name);
    return it->second;
}

// Загрузка текстуры
void ResourceManager::load(const std::string& name, const std::string& path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path))
        throw std::runtime_error("[ERROR]: Ошибка загрузки текстуры: " + path);
    texture.setSmooth(true);
    textures[name] = std::move(texture);
}

// Получение шрифта по имени
sf::Font& ResourceManager::getFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it == fonts.end())
        throw std::runtime_error("[ERROR]: Шрифт не найден: " + name);
    return it->second;
}

// Загрузка шрифта
void ResourceManager::loadFont(const std::string& name, const std::string& path) {
    sf::Font font;
    if (!font.openFromFile(path))
        throw std::runtime_error("[ERROR]: Ошибка загрузки шрифта: " + path);
    fonts[name] = std::move(font);
}
