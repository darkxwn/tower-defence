#include "GameData.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
std::map<EnemyType, EnemyStats> GameData::enemies;
std::map<std::string, TowerStats> GameData::towers;
std::vector<std::string> GameData::towerOrder;

// Вспомогательная функция для чтения файла из ресурсов SFML в строку
static std::string readFileToString(const std::string& path) {
    sf::FileInputStream stream;
    if (!stream.open(path)) {
        return "";
    }

    std::string content;
    auto size = stream.getSize();
    if (size) {
        content.resize(*size);
        stream.read(content.data(), *size);
    }
    return content;
}

void GameData::load() {
    enemies.clear();
    towers.clear();
    towerOrder.clear();

    // 1. ОПРЕДЕЛЯЕМ ПУТИ
#ifdef ANDROID
    std::string enemyPath = "config/enemies.cfg";
    std::string towerPath = "config/towers.cfg";
#else
    std::string enemyPath = "data/config/enemies.cfg";
    std::string towerPath = "data/config/towers.cfg";
#endif

    // 2. ЗАГРУЗКА ВРАГОВ
    std::string enemyData = readFileToString(enemyPath);
    if (!enemyData.empty()) {
        std::istringstream ssFile(enemyData);
        std::string line;
        while (std::getline(ssFile, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back(); // Чистим Windows-переносы
            if (line.empty()) continue;

            std::istringstream ss(line);
            std::string typeName;
            ss >> typeName;

            EnemyStats stats = {};
            std::string token;
            while (ss >> token) {
                size_t eq = token.find('=');
                if (eq == std::string::npos) continue;
                std::string key = token.substr(0, eq);
                int value = std::stoi(token.substr(eq + 1));

                if (key == "health")  stats.health = value;
                else if (key == "speed")   stats.speed = value;
                else if (key == "damage")  stats.damage = value;
                else if (key == "reward")  stats.reward = value;
            }

            EnemyType type;
            if (typeName == "basic")       type = EnemyType::Basic;
            else if (typeName == "fast")   type = EnemyType::Fast;
            else if (typeName == "strong") type = EnemyType::Strong;
            else continue;

            enemies[type] = stats;
        }
    }

    // 3. ЗАГРУЗКА БАШЕН
    std::string towerData = readFileToString(towerPath);
    if (!towerData.empty()) {
        std::istringstream ssFile(towerData);
        std::string line;
        while (std::getline(ssFile, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            std::istringstream ss(line);
            std::string name;
            ss >> name;

            TowerStats stats = {};
            std::string token;
            while (ss >> token) {
                size_t eq = token.find('=');
                if (eq == std::string::npos) continue;
                std::string key = token.substr(0, eq);
                std::string valStr = token.substr(eq + 1);

                if (key == "damage")   stats.damage = std::stoi(valStr);
                else if (key == "firerate") {
                    // Заменяем запятую на точку для stof, если конфиг был с запятой
                    std::replace(valStr.begin(), valStr.end(), ',', '.');
                    stats.firerate = std::stof(valStr);
                }
                else if (key == "range")    stats.range = std::stof(valStr);
                else if (key == "cost")     stats.cost = std::stoi(valStr);
                else if (key == "splash")   stats.splash = std::stoi(valStr);
            }

            towers[name] = stats;
            towerOrder.push_back(name);
        }
    }
}

// Метод получения статов врага по его типу
EnemyStats GameData::getEnemy(EnemyType type) {
    auto it = enemies.find(type);
    if (it == enemies.end())
        throw std::runtime_error("[Ошибка]: Статы врага не найдены");
    return it->second;
}

// Метод получения статов башни по ее типу
TowerStats GameData::getTower(const std::string& name) {
    auto it = towers.find(name);
    if (it == towers.end())
        throw std::runtime_error("[Ошибка]: Статы башни не найдены: " + name);
    return it->second;
}

std::vector<std::string> GameData::getTowerNames() { return towerOrder; }