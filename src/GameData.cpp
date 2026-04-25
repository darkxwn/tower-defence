#include "GameData.hpp"
#include "utils/FileReader.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

// Хранилище ресурсов
std::map<std::string, EnemyStats> GameData::enemies;
std::map<std::string, TowerStats> GameData::towers;
std::vector<std::string> GameData::towerOrder;
std::vector<std::string> GameData::enemyTypes;

// Загрузка данных из конфигурационных файлов
void GameData::load() {
    enemies.clear();
    towers.clear();
    towerOrder.clear();
    enemyTypes.clear();

#ifdef ANDROID
    std::string enemyPath = "config/enemies.cfg";
    std::string towerPath = "config/towers.cfg";
#else
    std::string enemyPath = "data/config/enemies.cfg";
    std::string towerPath = "data/config/towers.cfg";
#endif

    // загрузка врагов
    auto enemyData = readFile(enemyPath);
    std::istringstream ssFile(enemyData.value());
    std::string line;
    while (std::getline(ssFile, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue; // игнорируем пустые строки и комментарии

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

        // Автоматически добавляем новый тип врага в мапу и список имен
        enemies[typeName] = stats;
        enemyTypes.push_back(typeName);
    }

    // загрузка башен
    auto towerData = readFile(towerPath);
    std::istringstream ssFileTower(towerData.value());
    std::string lineTower;
    while (std::getline(ssFileTower, lineTower)) {
        if (!lineTower.empty() && lineTower.back() == '\r') lineTower.pop_back();
        if (lineTower.empty() || lineTower[0] == '#') continue;

        std::istringstream ss(lineTower);
        std::string name;
        ss >> name;

        TowerStats stats = {};
        stats.level = 0;
        stats.costDamage = 50;
        stats.costFirerate = 80;
        stats.costRange = 100;
        stats.costLevel = 200;
        std::string token;
        while (ss >> token) {
            size_t eq = token.find('=');
            if (eq == std::string::npos) continue;
            std::string key = token.substr(0, eq);
            std::string valStr = token.substr(eq + 1);

            if (key == "damage")   stats.damage = std::stoi(valStr);
            else if (key == "firerate") {
                std::replace(valStr.begin(), valStr.end(), ',', '.');
                stats.firerate = std::stof(valStr);
            }
            else if (key == "range")    stats.range = std::stof(valStr);
            else if (key == "cost")     stats.cost = std::stoi(valStr);
            else if (key == "splash")   stats.splash = std::stoi(valStr);
            else if (key == "level")    stats.level = std::stoi(valStr);
            else if (key == "costDamage")   stats.costDamage = std::stoi(valStr);
            else if (key == "costFirerate")  stats.costFirerate = std::stoi(valStr);
            else if (key == "costRange")    stats.costRange = std::stoi(valStr);
            else if (key == "costLevel")   stats.costLevel = std::stoi(valStr);
        }

        towers[name] = stats;
        towerOrder.push_back(name);
    }
}

// Получение характеристик врага по его строковому идентификатору (типу)
EnemyStats GameData::getEnemy(const std::string& type) {
    auto it = enemies.find(type);
    if (it == enemies.end())
        throw std::runtime_error("[Ошибка]: Статы врага не найдены: " + type);
    return it->second;
}

// Получение характеристик башни по имени
TowerStats GameData::getTower(const std::string& name) {
    auto it = towers.find(name);
    if (it == towers.end())
        throw std::runtime_error("[Ошибка]: Статы башни не найдены: " + name);
    return it->second;
}

// Получение списка всех доступных башен
std::vector<std::string> GameData::getTowerNames() {
    return towerOrder;
}

// Получение списка всех типов врагов для автоматической загрузки ресурсов
std::vector<std::string> GameData::getEnemyTypes() {
    return enemyTypes;
}

// Получение базовых характеристик башни для инициализации улучшений
TowerStats GameData::getBaseTowerStats(const std::string& name) {
    auto it = towers.find(name);
    if (it == towers.end())
        throw std::runtime_error("[Ошибка]: Статы башни не найдены: " + name);
    return it->second;
}
