#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class ResourceManager {
private:
    static std::map<std::string, sf::Texture> textures;

public:
    static sf::Texture& get(const std::string& name);
    static void load(const std::string& name, const std::string& path);

};