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

using Engine::Logger;

// Загрузка уровня из .map файла
void Map::load(const std::string& filePath) {
    // сброс данных перед загрузкой
    tiles.clear();
    path.clear();
    allowedEnemies.clear();
    starThresholds.clear();
    selectedTile = nullptr;
    width = height = startCoins = 0;
    levelName = "Без названия";
    portalPos = { -1, -1 };
    basePos = { -1, -1 };

    auto content = readFile(filePath);

    if (!content.has_value()) {
        Logger::error("Не удалось открыть файл карты: {}", filePath);
        return;
    }

    std::istringstream file(content.value());
    std::string line;

    // чтение метаданных
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#' || line[0] == '\r') continue;

        if (line.rfind("name=", 0) == 0) levelName = line.substr(5);
        else if (line.rfind("width=", 0) == 0) width = std::stoi(line.substr(6));
        else if (line.rfind("height=", 0) == 0) height = std::stoi(line.substr(7));
        else if (line.rfind("coins=", 0) == 0) startCoins = std::stoi(line.substr(6));
        else if (line.rfind("money=", 0) == 0) startCoins = std::stoi(line.substr(6)); // совместимость
        else if (line.rfind("enemies=", 0) == 0) {
            std::stringstream ss(line.substr(8));
            std::string type;
            while (ss >> type) allowedEnemies.push_back(type);
        }
        else if (line.rfind("stars=", 0) == 0) {
            std::stringstream ss(line.substr(6));
            int val;
            while (ss >> val) starThresholds.push_back(val);
        }
        else if (line == "tiles=") break;
    }

    // проверка корректности размеров
    if (width <= 0 || height <= 0) {
        Logger::error("[ERROR]: Некорректные размеры карты в файле {}", filePath);
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
        Logger::error("На карте {} не найден портал (1) или база (3)!", filePath);
    }

    for (auto& row : tiles) {
        for (auto& tile : row) {
            assignTileTexture(tile);
        }
    }

    portalLayer1Tex = &ResourceManager::get("portal-layer1");
    portalLayer2Tex = &ResourceManager::get("portal-layer2");
    activeTex = &ResourceManager::get("active");

    Logger::debug("Карта загружена. Игра успешно запущена!");
}

// Присваивание текстур тайлам
void Map::assignTileTexture(Tile& tile) {
    switch (tile.type) {
        case TileType::Road:     tile.texture = &ResourceManager::get("road"); break;
        case TileType::Platform: tile.texture = &ResourceManager::get("platform"); break;
        case TileType::Portal:   tile.texture = &ResourceManager::get("portal"); break;
        case TileType::Base:     tile.texture = &ResourceManager::get("base"); break;
        default:                 tile.texture = nullptr; break;
    }
}

// Обновление анимации портала
void Map::update(float dt) {
    portalAngle += 75.f * dt;
    if (portalAngle >= 360.f) portalAngle -= 360.f;
}

// Отрисовка карты
void Map::render(sf::RenderWindow& window, bool showSelected) {
    for (int y = 0; y < (int)tiles.size(); y++) {
        for (int x = 0; x < (int)tiles[y].size(); x++) {
            Tile& tile = tiles[y][x];

            // 1. Если текстуры нет, этот тайл пустой — пропускаем
            if (!tile.texture) continue;

            // 2. Рисуем основной тайл
            sf::Sprite sprite(*tile.texture);
            sprite.setScale({ 0.125f, 0.125f });
            sprite.setPosition(sf::Vector2f(tile.gridPos * 64) + mapOffset);
            window.draw(sprite);

            // 3. Если это портал — рисуем слои анимации ПОВЕРХ
            if (tile.type == TileType::Portal && portalLayer1Tex && portalLayer2Tex) {
                sf::Vector2f center = sprite.getPosition() + sf::Vector2f(32.f, 32.f);

                sf::Sprite l1(*portalLayer1Tex);
                l1.setOrigin({ 256.f, 256.f });
                l1.setScale({ 0.12f, 0.12f });
                l1.setPosition(center);
                l1.setRotation(sf::degrees(portalAngle));
                window.draw(l1);

                sf::Sprite l2(*portalLayer2Tex);
                l2.setOrigin({ 256.f, 256.f });
                l2.setScale({ 0.085f, 0.085f });
                l2.setPosition(center);
                l2.setRotation(sf::degrees(-portalAngle));
                window.draw(l2);
            }

            // 4. Подсветка выбора
            if (showSelected && selectedTile == &tile && activeTex) {
                sf::Sprite act(*activeTex);
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
    if (from[basePos.y][basePos.x] == sf::Vector2i{ -1, -1 }) {
        Logger::error("КРИТИЧЕСКАЯ ОШИБКА: Путь от портала до базы не найден!");
        return;
    }

    sf::Vector2i cur = basePos;
    while (cur != portalPos) {
        path.push_back(cur);
        cur = from[cur.y][cur.x];
        if (cur == sf::Vector2i{ -1, -1 }) break; // на всякий случай
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
int Map::getStartCoins() const {
    return startCoins;
}

// Получение названия уровня
std::string Map::getName() const {
    return levelName;
}

// Получение списка врагов
const std::vector<std::string>& Map::getAllowedEnemies() const {
    return allowedEnemies;
}

// Получение порогов звезд
const std::vector<int>& Map::getStarThresholds() const {
    return starThresholds;
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
