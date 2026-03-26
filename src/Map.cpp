#include "Map.hpp"
#include "ResourceManager.hpp"
#include "Tile.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>

// Метод загрузки уровня из .map файла
void Map::load(const std::string& filePath) {
    // Сбрасываем состояние перед загрузкой нового уровня
    tiles.clear();
    path.clear();
    selectedTile = nullptr;
    width = height = startMoney = 0;

    std::ifstream file(filePath);
    std::string line;

    // Читаем метаданные уровня
    while (std::getline(file, line)) {
        if (line.rfind("width=", 0) == 0)
            width = std::stoi(line.substr(6));
        if (line.rfind("height=", 0) == 0)
            height = std::stoi(line.substr(7));
        if (line.rfind("money=", 0) == 0)
            startMoney = std::stoi(line.substr(6));
        if (line == "tiles=")
            break;  // дальше идут тайлы
    }

    tiles.resize(height, std::vector<Tile>(width));

    // Считываем тайлы уровня
    for (int y = 0; y < height; y++) {
        std::getline(file, line);
        std::istringstream row(line);
        for (int x = 0; x < width; x++) {
            int typeId;
            row >> typeId;
            tiles[y][x].gridPos = { x, y };
            tiles[y][x].type = static_cast<TileType>(typeId);

            if (tiles[y][x].type == TileType::Portal)
                portalPos = { x, y };
            if (tiles[y][x].type == TileType::Base)
                basePos = { x, y };
        }
    }

    buildPath();
}

// Отрисовка карты и выбранного тайла
void Map::render(sf::RenderWindow& window) {
    for (int y = 0; y < (int)tiles.size(); y++) {
        for (int x = 0; x < (int)tiles[y].size(); x++) {
            Tile& tile = tiles[y][x];

            if (tile.type == TileType::Empty) continue;

            std::string texName;
            switch (tile.type) {
                case TileType::Road:     texName = "road";     break;
                case TileType::Platform: texName = "platform"; break;
                case TileType::Portal:   texName = "portal";   break;
                case TileType::Base:     texName = "base";     break;
                default: continue;
            }

            sf::Sprite sprite(ResourceManager::get(texName));
            sprite.setPosition(sf::Vector2f(tile.gridPos * 64) + mapOffset);
            window.draw(sprite);

            // Подсветка выбранного тайла
            if (selectedTile == &tile) {
                sf::Sprite active(ResourceManager::get("active"));
                active.setPosition(sprite.getPosition());
                window.draw(active);
            }
        }
    }
}

// Центрирует карту в доступной области экрана
void Map::centerOnScreen(sf::Vector2u windowSize, float topPanelHeight, float bottomPanelHeight) {
    float mapPixelW = width * 64.f;
    float mapPixelH = height * 64.f;
    float availableH = windowSize.y - (topPanelHeight + bottomPanelHeight);

    mapOffset.x = (windowSize.x - mapPixelW) / 2.f;
    mapOffset.y = topPanelHeight + (availableH - mapPixelH) / 2.f;
}

// Поиск пути движения врагов (BFS от портала до базы)
void Map::buildPath() {
    std::queue<sf::Vector2i> queue;
    std::vector<std::vector<sf::Vector2i>> cameFrom(
        height, std::vector<sf::Vector2i>(width, { -1, -1 })
    );

    queue.push(portalPos);

    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();

        if (current == basePos) break;

        std::vector<sf::Vector2i> dirs = { {0,-1},{0,1},{-1,0},{1,0} };
        for (auto& dir : dirs) {
            sf::Vector2i neighbor = { current.x + dir.x, current.y + dir.y };

            if (neighbor.x < 0 || neighbor.x >= width ||
                neighbor.y < 0 || neighbor.y >= height) continue;

            TileType t = tiles[neighbor.y][neighbor.x].type;
            if (t != TileType::Road && t != TileType::Base && t != TileType::Portal) continue;

            if (cameFrom[neighbor.y][neighbor.x] != sf::Vector2i{ -1, -1 }) continue;

            cameFrom[neighbor.y][neighbor.x] = current;
            queue.push(neighbor);
        }
    }

    // Восстанавливаем путь от Base к Portal и разворачиваем
    sf::Vector2i current = basePos;
    while (current != portalPos) {
        path.push_back(current);
        current = cameFrom[current.y][current.x];
    }
    path.push_back(portalPos);
    std::reverse(path.begin(), path.end());
}

// Возвращает указатель на тайл под экранными координатами мыши
Tile* Map::getTileAtScreen(sf::Vector2f screenPos) const {
    sf::Vector2i gridPos = sf::Vector2i((screenPos - mapOffset) / 64.f);

    if (gridPos.x < 0 || gridPos.x >= width ||
        gridPos.y < 0 || gridPos.y >= height)
        return nullptr;

    return const_cast<Tile*>(&tiles[gridPos.y][gridPos.x]);
}

const std::vector<sf::Vector2i>& Map::getPath() const    { return path; }
sf::Vector2i Map::getBasePos() const                      { return basePos; }
int Map::getStartMoney() const                            { return startMoney; }
sf::Vector2f Map::getMapOffset() const                    { return mapOffset; }

void Map::setSelectedTile(sf::Vector2f screenPos) {
    Tile* tile = getTileAtScreen(screenPos);
    selectedTile = (tile && tile->type == TileType::Platform) ? tile : nullptr;
}

Tile* Map::getSelectedTile() const { return selectedTile; }
