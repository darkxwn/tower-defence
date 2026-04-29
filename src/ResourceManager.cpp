#include "ResourceManager.hpp"
#include "utils/Logger.hpp"

using Engine::Logger;

std::map<std::string, sf::Texture> ResourceManager::textures;
std::map<std::string, sf::Font> ResourceManager::fonts;

// Получение текстуры по имени
sf::Texture& ResourceManager::get(const std::string& name) {
    auto it = textures.find(name);
    if (it == textures.end()) {
        Logger::error("Текстура не найдена: {}", name);
        throw std::runtime_error("[ERROR]: Текстура не найдена: " + name);
    }
    // Logger::debug("Текстура получена: {}", name);
    return it->second;
}

// Загрузка текстуры
void ResourceManager::load(const std::string& name, const std::string& path, bool smooth) {
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        Logger::error("Ошибка загрузки текстуры: {}", path);
        throw std::runtime_error("[ERROR]: Ошибка загрузки текстуры: " + path);
    }
    texture.setSmooth(smooth);
    textures[name] = std::move(texture);
    Logger::debug("Текстура загружена: {}", path);
}

// Получение шрифта по имени
sf::Font& ResourceManager::getFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it == fonts.end()) {
        Logger::error("Шрифт не найден: {}", name);
        throw std::runtime_error("[ERROR]: Шрифт не найден: " + name);
    }
    // Logger::debug("Шрифт получен: {}", name);
    return it->second;
}

// Загрузка шрифта
void ResourceManager::loadFont(const std::string& name, const std::string& path) {
    sf::Font font;
    if (!font.openFromFile(path)) {
        Logger::error("Ошибка загрузки шрифта: {}", path);
        throw std::runtime_error("[ERROR]: Ошибка загрузки шрифта: " + path);
    }
    fonts[name] = std::move(font);
    Logger::debug("Шрифт загружен: {}", path);
}
