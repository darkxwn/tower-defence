#include "Map.hpp"
#include "ResourceManager.hpp"
#include "Tile.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue> 
#include <sstream>
#include <string>

Map::Map() {
    load("data/levels/level01.map");
}

// Метод загрузки уровня из файла 
void Map::load(const std::string& path) {
    std::ifstream file(path);
    std::string line;

    // сначала читаем width и height
    while (std::getline(file, line)) {
        if (line.rfind("width=", 0) == 0)
            width = std::stoi(line.substr(6));
        if (line.rfind("height=", 0) == 0)
            height = std::stoi(line.substr(7));
        if (line.rfind("money=", 0) == 0)
            startMoney = std::stoi(line.substr(6));
        if (line == "tiles=")
            break;  // остановились — дальше идут тайлы
    }

    tiles.resize(height, std::vector<Tile>(width)); // Cоздаём карту width x height

    // Считываем позицию и тип тайлов
    for (int y = 0; y < height; y++) {
        std::getline(file, line);
        std::istringstream row(line);  // разбиваем строку на числа
        for (int x = 0; x < width; x++) {
            int typeId;
            row >> typeId;  // читаем следующее число
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

void Map::render(sf::RenderWindow& window) {
    for (int y = 0; y < tiles.size(); y++) {
        for (int x = 0; x < tiles[y].size(); x++) {
            Tile& tile = tiles[y][x];

            if (tile.type == TileType::Empty) continue;

            // выбираем имя текстуры по типу
            std::string texName;
            switch (tile.type) {
                case TileType::Road:       
                    texName = "road";      
                    break;
                case TileType::Platform:   
                    texName = "platform";   
                    break;
                case TileType::Portal: 
                    texName = "portal"; 
                    break;
                case TileType::Base:   
                    texName = "base";   
                    break;
                default: 
                    continue;
            }

            sf::Sprite sprite(ResourceManager::get(texName));
            sprite.setPosition(sf::Vector2f(tile.gridPos * 64) + mapOffset);
            window.draw(sprite);

            // если этот тайл выбран — рисуем обводку поверх
            if (selectedTile == &tile) {
                sf::Sprite active(ResourceManager::get("active"));
                active.setPosition(sprite.getPosition());
                window.draw(active);
            }
        }
    }
}

void Map::centerOnScreen(sf::Vector2u windowSize, float topPanelHeight, float bottomPanelHeight) {
    float mapPixelW = width * 64.f;
    float mapPixelH = height * 64.f;

    // доступная область — весь экран минус нижняя панель
    float availableH = windowSize.y - (topPanelHeight + bottomPanelHeight);

    mapOffset.x = (windowSize.x - mapPixelW) / 2.f;
    mapOffset.y = topPanelHeight + (availableH - mapPixelH) / 2.f;
}

// Метод поиска пути движения врагов (BFS) для загруженной карты
void Map::buildPath() {
    std::queue<sf::Vector2i> queue;      // Очередь тайлов для проверки
    std::vector<std::vector<sf::Vector2i>> cameFrom(
        height, std::vector<sf::Vector2i>(width, { -1, -1 })
    );  // Откуда пришли

    queue.push(portalPos);

    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();

        if (current == basePos) break;  // нашли!

        // проверяем 4 соседей
        std::vector<sf::Vector2i> dirs = { {0,-1},{0,1},{-1,0},{1,0} };

        for (auto& dir : dirs) {
            sf::Vector2i neighbor = { current.x + dir.x, current.y + dir.y };

            // проверка границ
            if (neighbor.x < 0 || neighbor.x >= width ||
                neighbor.y < 0 || neighbor.y >= height) continue;

            // проверка типа тайла
            TileType t = tiles[neighbor.y][neighbor.x].type;
            if (t != TileType::Road && t != TileType::Base && t != TileType::Portal) continue;

            // проверка не посещён ли
            if (cameFrom[neighbor.y][neighbor.x] != sf::Vector2i{ -1, -1 }) continue;

            cameFrom[neighbor.y][neighbor.x] = current;
            queue.push(neighbor);
        }
    }

    // восстанавливаем путь от Base до Portal
    sf::Vector2i current = basePos;
    while (current != portalPos) {
        path.push_back(current);
        current = cameFrom[current.y][current.x];
    }
    path.push_back(portalPos);

    // разворачиваем — сейчас путь идёт от Base к Portal
    std::reverse(path.begin(), path.end());
}

// Получение позиции тайла в зависимости от клика мышью
Tile* Map::getTileAtScreen(sf::Vector2f screenPos) const {
    // переводим экранные координаты в координаты сетки
    sf::Vector2i gridPos = sf::Vector2i((screenPos - mapOffset) / 64.f);

    if (gridPos.x < 0 || gridPos.x >= width ||
        gridPos.y < 0 || gridPos.y >= height)
        return nullptr;

    return const_cast<Tile*>(&tiles[gridPos.y][gridPos.x]);
}

// Метод получения пути движения врагов
const std::vector<sf::Vector2i>& Map::getPath() const {
    return path;
}

// Метод получения позиции тайла базы
sf::Vector2i Map::getBasePos() const {
    return basePos;
}

int Map::getStartMoney() const { return startMoney; }
sf::Vector2f Map::getMapOffset() const { return mapOffset; }

void Map::setSelectedTile(sf::Vector2f screenPos) {
    Tile* tile = getTileAtScreen(screenPos);
    if (tile && tile->type == TileType::Platform)
        selectedTile = tile;
    else
        selectedTile = nullptr;
}