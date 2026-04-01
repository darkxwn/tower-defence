#include "Map.hpp"
#include "ResourceManager.hpp"
#include <algorithm>
#include <fstream>
#include <queue>
#include <sstream>

void Map::load(const std::string& filePath) {
    // Сброс перед загрузкой нового уровня
    tiles.clear();
    path.clear();
    selectedTile = nullptr;
    width = height = startMoney = 0;

    std::ifstream file(filePath);
    std::string line;

    // Метаданные
    while (std::getline(file, line)) {
        if (line.rfind("width=",  0) == 0) width       = std::stoi(line.substr(6));
        if (line.rfind("height=", 0) == 0) height      = std::stoi(line.substr(7));
        if (line.rfind("money=",  0) == 0) startMoney  = std::stoi(line.substr(6));
        if (line == "tiles=") break;
    }

    tiles.resize(height, std::vector<Tile>(width));

    for (int y = 0; y < height; y++) {
        std::getline(file, line);
        std::istringstream row(line);
        for (int x = 0; x < width; x++) {
            int id; row >> id;
            tiles[y][x].gridPos = {x, y};
            tiles[y][x].type    = static_cast<TileType>(id);
            if (tiles[y][x].type == TileType::Portal) portalPos = {x, y};
            if (tiles[y][x].type == TileType::Base)   basePos   = {x, y};
        }
    }

    buildPath();
}

void Map::render(sf::RenderWindow& window) {
    for (int y = 0; y < (int)tiles.size(); y++) {
        for (int x = 0; x < (int)tiles[y].size(); x++) {
            Tile& tile = tiles[y][x];
            if (tile.type == TileType::Empty) continue;

            std::string tex;
            std::string texName;
                case TileType::Road:     tex = "road";     break;
                case TileType::Platform: tex = "platform"; break;
                case TileType::Portal:   tex = "portal";   break;
                case TileType::Base:     tex = "base";     break;
                default: continue;
                    texName = "platform";   
                    break;
                case TileType::Portal: 
                    texName = "portal"; 
                    break;
            }
            sf::Sprite sp(ResourceManager::get(tex));
            sp.setPosition(sf::Vector2f(tile.gridPos * 64) + mapOffset);
            window.draw(sp);

            sf::Sprite sprite(ResourceManager::get(texName));
            sprite.setPosition(sf::Vector2f(tile.gridPos * 64) + mapOffset);
            window.draw(sprite);

void Map::centerOnScreen(sf::Vector2u ws, float topH, float botH) {
    float availH = ws.y - (topH + botH);
    mapOffset.x  = (ws.x - width  * 64.f) / 2.f;
    mapOffset.y  = topH + (availH - height * 64.f) / 2.f;
}

void Map::centerOnScreen(sf::Vector2u windowSize, float topPanelHeight, float bottomPanelHeight) {
    std::queue<sf::Vector2i> q;
    std::vector<std::vector<sf::Vector2i>> from(
        height, std::vector<sf::Vector2i>(width, {-1, -1}));

    q.push(portalPos);

    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        if (cur == basePos) break;
        auto current = queue.front();
        for (auto& d : std::vector<sf::Vector2i>{{0,-1},{0,1},{-1,0},{1,0}}) {
            sf::Vector2i nb = {cur.x + d.x, cur.y + d.y};
            if (nb.x < 0 || nb.x >= width || nb.y < 0 || nb.y >= height) continue;
            TileType t = tiles[nb.y][nb.x].type;
        if (current == basePos) break;  // нашли!
            if (from[nb.y][nb.x] != sf::Vector2i{-1,-1}) continue;
            from[nb.y][nb.x] = cur;
            q.push(nb);
            sf::Vector2i neighbor = { current.x + dir.x, current.y + dir.y };

            // проверка границ
    sf::Vector2i cur = basePos;
    while (cur != portalPos) {
        path.push_back(cur);
        cur = from[cur.y][cur.x];
            if (t != TileType::Road && t != TileType::Base && t != TileType::Portal) continue;

            // проверка не посещён ли
            if (cameFrom[neighbor.y][neighbor.x] != sf::Vector2i{ -1, -1 }) continue;

Tile* Map::getTileAtScreen(sf::Vector2f sp) const {
    sf::Vector2i gp = sf::Vector2i((sp - mapOffset) / 64.f);
    if (gp.x < 0 || gp.x >= width || gp.y < 0 || gp.y >= height) return nullptr;
    return const_cast<Tile*>(&tiles[gp.y][gp.x]);
}
    }
void Map::setSelectedTile(sf::Vector2f sp) {
    Tile* t = getTileAtScreen(sp);
    selectedTile = (t && t->type == TileType::Platform) ? t : nullptr;

    if (gridPos.x < 0 || gridPos.x >= width ||
const std::vector<sf::Vector2i>& Map::getPath()      const { return path; }
sf::Vector2i  Map::getBasePos()    const { return basePos; }
int           Map::getStartMoney() const { return startMoney; }
sf::Vector2f  Map::getMapOffset()  const { return mapOffset; }
Tile*         Map::getSelectedTile() const { return selectedTile; }
}

// Метод получения пути движения врагов
const std::vector<sf::Vector2i>& Map::getPath() const {
    return path;
}

// Метод получения позиции тайла базы
sf::Vector2i Map::getBasePos() const {
    return basePos;
}

// Метод получения стратового количества денег
int Map::getStartMoney() const { return startMoney; }

// Метод получения текущего отступа карты
sf::Vector2f Map::getMapOffset() const { return mapOffset; }

// Метод установки текущего выбранного тайла
void Map::setSelectedTile(sf::Vector2f screenPos) {
    Tile* tile = getTileAtScreen(screenPos);
    if (tile && tile->type == TileType::Platform)
        selectedTile = tile;
    else
        selectedTile = nullptr;
}

// Метод получения текущего выбранного тайла
Tile* Map::getSelectedTile() const {
    return selectedTile;
}