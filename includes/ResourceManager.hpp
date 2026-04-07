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
    static std::map<std::string, sf::Texture> textures;
    static std::map<std::string, sf::Font> fonts;

public:
    static sf::Texture& get(const std::string& name);
    static void load(const std::string& name, const std::string& path);

    static sf::Font& getFont(const std::string& name);
    static void loadFont(const std::string& name, const std::string& path);
};