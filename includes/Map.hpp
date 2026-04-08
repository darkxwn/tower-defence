#pragma once
#include <vector>
#include <string>
#include "Tile.hpp"

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС MAP
//
///////////////////////////////////////////////////////////////////////////

class Map {
private:
    int width = 0;
    int height = 0;
    int startMoney = 0;

    std::vector<std::vector<Tile>> tiles;
    Tile* selectedTile = nullptr;

    float portalAngle = 0.f; // Угол для вращения слоя

    std::vector<sf::Vector2i> path;
    sf::Vector2i portalPos;
    sf::Vector2i basePos;
    sf::Vector2f mapOffset;

    void buildPath();

public:
    Map() = default;
    void load(const std::string& filePath); // загружает уровень из .map файла
    
    void update(float dt);

    void render(sf::RenderWindow& window);
    void centerOnScreen(sf::Vector2u windowSize, float topPanelH, float bottomPanelH);

    void  setSelectedTile(sf::Vector2f screenPos);
    Tile* getTileAtScreen(sf::Vector2f screenPos) const;
    Tile* getSelectedTile() const;

    const std::vector<sf::Vector2i>& getPath()       const;
    sf::Vector2i  getBasePos()    const;
    int           getStartMoney() const;
    sf::Vector2f  getMapOffset()  const;

    int getWidth() const;
    int getHeight() const;
};
