#include "GameData.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

std::map<EnemyType, EnemyStats> GameData::enemies;
std::map<std::string, TowerStats> GameData::towers;
std::vector<std::string> GameData::towerOrder;

void GameData::load() {
    // загрузка врагов 
    std::ifstream enemyFile("data/config/enemies.cfg");
    std::string line;

    while (std::getline(enemyFile, line)) {
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string typeName;
        ss >> typeName;  // читаем имя типа — "basic", "fast" и т.д.

        EnemyStats stats = {};
        std::string token;

        // парсим пары ключ=значение
        while (ss >> token) {
            size_t eq = token.find('=');
            if (eq == std::string::npos) continue;
            std::string key = token.substr(0, eq);
            int value = std::stoi(token.substr(eq + 1));

            if (key == "health")  stats.health = value;
            if (key == "speed")   stats.speed = value;
            if (key == "damage")  stats.damage = value;
            if (key == "reward")  stats.reward = value;
        }

        // конвертируем строку в EnemyType
        EnemyType type;
        if (typeName == "basic")       type = EnemyType::Basic;
        else if (typeName == "fast")   type = EnemyType::Fast;
        else if (typeName == "strong") type = EnemyType::Strong;
        else continue;

        enemies[type] = stats;
    }


    // загрузка башен
    std::ifstream towerFile("data/config/towers.cfg");

    while (std::getline(towerFile, line)) {
        if (line.empty()) continue;

        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        std::istringstream ss(line);
        std::string name;
        ss >> name;  // имя башни — "basic", "cannon" и т.д.

        TowerStats stats = {};
        std::string token;

        while (ss >> token) {
            size_t eq = token.find('=');
            if (eq == std::string::npos) continue;
            std::string key = token.substr(0, eq);

            if (key == "damage")   stats.damage = std::stoi(token.substr(eq + 1));
            if (key == "firerate") { // заменяем запятую на точку на случай локали
                std::string val = token.substr(eq + 1);
                std::replace(val.begin(), val.end(), '.', ',');
                stats.firerate = std::stof(val);
            }           
            if (key == "range")    stats.range = std::stof(token.substr(eq + 1));
            if (key == "cost")     stats.cost = std::stoi(token.substr(eq + 1));
            if (key == "splash")   stats.splash = std::stoi(token.substr(eq + 1));
        }

        towers[name] = stats;
        towerOrder.push_back(name);
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