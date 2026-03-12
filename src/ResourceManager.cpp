#include "ResourceManager.hpp"

std::map<std::string, sf::Texture> ResourceManager::textures;

sf::Texture& ResourceManager::get(const std::string& name) {
    auto it = textures.find(name);
    if (it == textures.end()) {
        throw std::runtime_error("[Ошибка]: Не удалось получить текстуру  " + name);
    }
    return it->second;
}

void ResourceManager::load(const std::string& name, const std::string& path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        throw std::runtime_error("[Ошибка]: Ошибка загрузки текстуры из файла " + path);
    }
    textures[name] = std::move(texture);
}