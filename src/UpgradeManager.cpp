#include "UpgradeManager.hpp"
#include "GameData.hpp"
#include <random>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС UPGRADEMANAGER
//
///////////////////////////////////////////////////////////////////////////

int UpgradeManager::moneyMin = 1;
int UpgradeManager::moneyMax = 10;

// Установка callback для сохранения при изменении улучшения
void UpgradeManager::setSaveCallback(std::function<void(const TowerUpgrade&)> callback) {
    onUpgradeChanged = callback;
}

// Инициализация базовыми значениями из GameData
void UpgradeManager::initDefaults() {
    upgrades.clear();
    auto towerNames = GameData::getTowerNames();
    for (const auto& name : towerNames) {
        auto stats = GameData::getBaseTowerStats(name);
        upgrades.push_back({
            name,
            (float)stats.damage,
            stats.firerate,
            stats.range,
            stats.level,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            stats.costDamage,
            stats.costFirerate,
            stats.costRange,
            stats.costLevel
        });
    }
}

// Получение всех улучшений
const std::vector<UpgradeManager::TowerUpgrade>& UpgradeManager::getAllUpgrades() const {
    return upgrades;
}

// Установка всех улучшений
void UpgradeManager::setAllUpgrades(const std::vector<TowerUpgrade>& data) {
    upgrades = data;
}

// Получение улучшения для типа башни
const UpgradeManager::TowerUpgrade* UpgradeManager::getUpgrade(const std::string& towerType) const {
    for (const auto& up : upgrades) {
        if (up.towerType == towerType) {
            return &up;
        }
    }
    return nullptr;
}

// Получение актуального урона
float UpgradeManager::getDamage(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) {
        return up->baseDamage * up->damageMultiplier;
    }
    return 35.f;
}

// Получение актуальной скорости
float UpgradeManager::getFirerate(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) {
        return up->baseFirerate * up->firerateMultiplier;
    }
    return 1.0f;
}

// Получение актуальной дальности
float UpgradeManager::getRange(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) {
        return up->baseRange * up->rangeMultiplier;
    }
    return 192.f;
}

// Получение уровня башни
int UpgradeManager::getLevel(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) {
        return up->level;
    }
    return 0;
}

// Получение множителя денег
float UpgradeManager::getMoneyMultiplier(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) {
        return up->moneyMultiplier;
    }
    return 1.0f;
}

// Получение текущей стоимости улучшения
int UpgradeManager::getUpgradeCost(const std::string& towerType, int statIndex) const {
    if (const auto* up = getUpgrade(towerType)) {
        int baseCost = 0;
        float multiplier = 1.0f;
        
        if (statIndex == 0) { baseCost = up->costLevel; multiplier = 1.0f + up->level * 0.1f; }
        else if (statIndex == 1) { baseCost = up->costDamage; multiplier = up->damageMultiplier; }
        else if (statIndex == 2) { baseCost = up->costFirerate; multiplier = up->firerateMultiplier; }
        else if (statIndex == 3) { baseCost = up->costRange; multiplier = up->rangeMultiplier; }
        
        // формула: baseCost * (1 + (multiplier - 1) * 3)
        float price = baseCost * (1.0f + (multiplier - 1.0f) * 3.0f);
        return (int)price;
    }
    return 100;
}

// Получение текущей стоимости улучшения по ключу
int UpgradeManager::getUpgradeCost(const std::string& towerType, const std::string& statKey) const {
    static const std::unordered_map<std::string, int> keyToIndex = {
        {"level", 3},
        {"damage", 1},
        {"firerate", 2},
        {"range", 0}
    };
    auto it = keyToIndex.find(statKey);
    if (it != keyToIndex.end()) {
        return getUpgradeCost(towerType, it->second);
    }
    return 100;
}

// Получение случайного количества денег за убийство
int UpgradeManager::getRandomMoney() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(moneyMin, moneyMax);
    return dis(gen);
}

// Изменение улучшения урона
void UpgradeManager::upgradeDamage(const std::string& towerType, float increment) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType) {
            up.damageMultiplier += increment;
            if (onUpgradeChanged) {
                onUpgradeChanged(up);
            }
            break;
        }
    }
}

// Изменение улучшения скорости
void UpgradeManager::upgradeFirerate(const std::string& towerType, float increment) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType) {
            up.firerateMultiplier += increment;
            if (onUpgradeChanged) {
                onUpgradeChanged(up);
            }
            break;
        }
    }
}

// Изменение улучшения дальности
void UpgradeManager::upgradeRange(const std::string& towerType, float increment) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType) {
            up.rangeMultiplier += increment;
            if (onUpgradeChanged) {
                onUpgradeChanged(up);
            }
            break;
        }
    }
}

// Изменение уровня башни
void UpgradeManager::upgradeLevel(const std::string& towerType) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType && up.level < (int)MAX_TOWER_LEVEL) {
            up.level++;
            // увеличиваем множители при повышении уровня
            up.damageMultiplier += 0.01f;
            up.firerateMultiplier += 0.01f;
            up.rangeMultiplier += 0.01f;
            up.moneyMultiplier += 0.01f;
            if (onUpgradeChanged) {
                onUpgradeChanged(up);
            }
            break;
        }
    }
}