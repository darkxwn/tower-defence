#include "Map.hpp"
#include "ResourceManager.hpp"
#include "utils/FileReader.hpp"
#include "utils/Logger.hpp"
#include <SFML/System/FileInputStream.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>

// Загрузка уровня из .map файла
void Map::load(const std::string& filePath) {
    // сброс данных перед загрузкой
    tiles.clear();
    path.clear();
    selectedTile = nullptr;
    width = height = startMoney = 0;
    portalPos = { -1, -1 };
    basePos = { -1, -1 };

    auto content = readFile(filePath);

    if (!content.has_value()) {
        LOGI("[ERROR]: Не удалось открыть файл карты: %s", filePath.c_str());
        return;
    }

    std::istringstream file(content.value());
    std::string line;

    // чтение метаданных
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        if (line.rfind("width=", 0) == 0) width = std::stoi(line.substr(6));
        else if (line.rfind("height=", 0) == 0) height = std::stoi(line.substr(7));
        else if (line.rfind("money=", 0) == 0) startMoney = std::stoi(line.substr(6));
        else if (line == "tiles=") break;
    }

    // проверка корректности размеров
    if (width <= 0 || height <= 0) {
        std::cerr << "[ERROR]: Некорректные размеры карты в файле " << filePath << std::endl;
        return;
    }

    tiles.resize(height, std::vector<Tile>(width));

    // чтение сетки тайлов
    for (int y = 0; y < height; y++) {
        if (!std::getline(file, line)) break;
        std::istringstream row(line);
        for (int x = 0; x < width; x++) {
            int id;
            if (!(row >> id)) id = 0;

            tiles[y][x].gridPos = { x, y };
            tiles[y][x].type = static_cast<TileType>(id);
            if (tiles[y][x].type == TileType::Portal) portalPos = { x, y };
            if (tiles[y][x].type == TileType::Base)   basePos = { x, y };
        }
    }

    // проверка наличия портала и базы
    if (portalPos.x != -1 && basePos.x != -1) {
        buildPath();
    } else {
        std::cerr << "[ERROR]: На карте " << filePath << " не найден портал (1) или база (3)!" << std::endl;
    }
}

// Обновление анимации портала
void Map::update(float dt) {
    portalAngle += 75.f * dt;
    if (portalAngle >= 360.f) portalAngle -= 360.f;
}

// Отрисовка карты
void Map::render(sf::RenderWindow& window) {
    for (int y = 0; y < (int)tiles.size(); y++) {
        for (int x = 0; x < (int)tiles[y].size(); x++) {
            Tile& tile = tiles[y][x];
            if (tile.type == TileType::Empty) continue;

            // выбор текстуры по типу тайла
            std::string tex;
            switch (tile.type) {
            case TileType::Road:
                tex = "road";
                break;
            case TileType::Platform:
                tex = "platform";
                break;
            case TileType::Portal:
                tex = "portal";
                break;
            case TileType::Base:
                tex = "base";
                break;
            default:
                continue;
            }

            // отрисовка основного тайла
            sf::Sprite sprite(ResourceManager::get(tex));
            sprite.setScale({ 0.125f, 0.125f });
            sprite.setPosition(sf::Vector2f(tile.gridPos * 64) + mapOffset);
            window.draw(sprite);

            // анимация портала
            if (tile.type == TileType::Portal) {
                sf::Vector2f center = sf::Vector2f(tile.gridPos * 64) + mapOffset + sf::Vector2f(32.f, 32.f);

                sf::Sprite layer1(ResourceManager::get("portal-layer1"));
                layer1.setOrigin({ 256.f, 256.f });
                layer1.setScale({ 0.12f, 0.12f });
                layer1.setPosition(center);
                layer1.setRotation(sf::degrees(portalAngle));

                sf::Sprite layer2(ResourceManager::get("portal-layer2"));
                layer2.setOrigin({ 256.f, 256.f });
                layer2.setScale({ 0.085f, 0.085f });
                layer2.setPosition(center);
                layer2.setRotation(sf::degrees(-portalAngle));

                window.draw(layer1);
                window.draw(layer2);
            }

            // подсветка выбранного тайла
            if (selectedTile == &tile) {
                sf::Sprite act(ResourceManager::get("active"));
                act.setScale({ 0.125f, 0.125f });
                act.setPosition(sprite.getPosition());
                window.draw(act);
            }
        }
    }
}

// Центрирование карты на экране
void Map::centerOnScreen(sf::Vector2u ws, float topH, float botH) {
    float availH = ws.y - (topH + botH);
    mapOffset.x = (ws.x - width * 64.f) / 2.f;
    mapOffset.y = topH + (availH - height * 64.f) / 2.f;
}

// Построение пути от портала к базе
void Map::buildPath() {
    std::queue<sf::Vector2i> q;
    std::vector<std::vector<sf::Vector2i>> from(
        height, std::vector<sf::Vector2i>(width, { -1, -1 }));

    q.push(portalPos);

    // поиск в ширину от портала к базе
    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        if (cur == basePos) break;

        for (auto& d : std::vector<sf::Vector2i>{ {0,-1},{0,1},{-1,0},{1,0} }) {
            sf::Vector2i nb = { cur.x + d.x, cur.y + d.y };
            if (nb.x < 0 || nb.x >= width || nb.y < 0 || nb.y >= height) continue;
            TileType t = tiles[nb.y][nb.x].type;
            if (t != TileType::Road && t != TileType::Base && t != TileType::Portal) continue;
            if (from[nb.y][nb.x] != sf::Vector2i{ -1,-1 }) continue;
            from[nb.y][nb.x] = cur;
            q.push(nb);
        }
    }

    // восстановление пути от базы к порталу
    sf::Vector2i cur = basePos;
    while (cur != portalPos) {
        path.push_back(cur);
        cur = from[cur.y][cur.x];
    }
    path.push_back(portalPos);
    std::reverse(path.begin(), path.end());
}

// Тайл по экранным координатам
Tile* Map::getTileAtScreen(sf::Vector2f sprite) const {
    sf::Vector2i gp = sf::Vector2i((sprite - mapOffset) / 64.f);
    if (gp.x < 0 || gp.x >= width || gp.y < 0 || gp.y >= height) return nullptr;
    return const_cast<Tile*>(&tiles[gp.y][gp.x]);
}

// Выбор тайла по экранным координатам
void Map::setSelectedTile(sf::Vector2f sprite) {
    Tile* t = getTileAtScreen(sprite);
    selectedTile = (t && t->type == TileType::Platform) ? t : nullptr;
}

// Получение пути врагов
const std::vector<sf::Vector2i>& Map::getPath() const {
    return path;
}

// Получение позиции базы
sf::Vector2i Map::getBasePos() const {
    return basePos;
}

// Получение стартовых денег
int Map::getStartMoney() const {
    return startMoney;
}

// Получение смещения карты
sf::Vector2f Map::getMapOffset() const {
    return mapOffset;
}

// Получение выбранного тайла
Tile* Map::getSelectedTile() const {
    return selectedTile;
}

// Получение ширины карты
int Map::getWidth() const {
    return width;
}

// Получение высоты карты
int Map::getHeight() const {
    return height;
}
