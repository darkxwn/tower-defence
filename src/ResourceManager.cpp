#include "ResourceManager.hpp"

std::map<std::string, sf::Texture> ResourceManager::textures;
std::map<std::string, sf::Font> ResourceManager::fonts;

sf::Texture& ResourceManager::get(const std::string& name) {
    auto it = textures.find(name);
    if (it == textures.end())
        throw std::runtime_error("[Ошибка]: Текстура не найдена: " + name);
    return it->second;
}

void ResourceManager::load(const std::string& name, const std::string& path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path))
        throw std::runtime_error("[Ошибка]: Ошибка загрузки текстуры: " + path);
    texture.setSmooth(true);
    textures[name] = std::move(texture);
}

sf::Font& ResourceManager::getFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it == fonts.end())
        throw std::runtime_error("[Ошибка]: Шрифт не найден: " + name);
    return it->second;
}

void ResourceManager::loadFont(const std::string& name, const std::string& path) {
    sf::Font font;
    if (!font.openFromFile(path))
        throw std::runtime_error("[Ошибка]: Ошибка загрузки шрифта: " + path);
    fonts[name] = std::move(font);
}