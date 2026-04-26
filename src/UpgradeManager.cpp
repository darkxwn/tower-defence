#include "UpgradeManager.hpp"
#include "GameData.hpp"
#include <utils/Math.hpp>
#include <unordered_map>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС UPGRADEMANAGER
//
///////////////////////////////////////////////////////////////////////////

int UpgradeManager::moneyMin = 1;
int UpgradeManager::moneyMax = 10;

void UpgradeManager::setSaveCallback(std::function<void()> callback) {
    onUpgradeChanged = callback;
}

void UpgradeManager::initDefaults() {
    upgrades.clear();
    auto towerNames = GameData::getTowerNames();
    for (const auto& name : towerNames) {
        auto stats = GameData::getBaseTowerStats(name);
        TowerUpgrade up;
        up.towerType = name;
        up.baseDamage = (float)stats.damage;
        up.baseFirerate = stats.firerate;
        up.baseRange = stats.range;
        up.rank = stats.rank;
        up.level = stats.level;
        up.damageLvl = 0;
        up.firerateLvl = 0;
        up.rangeLvl = 0;
        up.damageMultiplier = 1.0f;
        up.firerateMultiplier = 1.0f;
        up.rangeMultiplier = 1.0f;
        up.costRank = stats.costRank;
        up.costDamage = stats.costDamage;
        up.costFirerate = stats.costFirerate;
        up.costRange = stats.costRange;
        up.costLevel = stats.costLevel;
        
        upgrades.push_back(up);
    }
}

const std::vector<UpgradeManager::TowerUpgrade>& UpgradeManager::getAllUpgrades() const {
    return upgrades;
}

void UpgradeManager::setAllUpgrades(const std::vector<TowerUpgrade>& data) {
    upgrades = data;
}

const UpgradeManager::TowerUpgrade* UpgradeManager::getUpgrade(const std::string& towerType) const {
    for (const auto& up : upgrades) {
        if (up.towerType == towerType) return &up;
    }
    return nullptr;
}

float UpgradeManager::getDamage(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) return up->baseDamage * up->damageMultiplier;
    return 35.f;
}

float UpgradeManager::getFirerate(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) return up->baseFirerate * up->firerateMultiplier;
    return 1.0f;
}

float UpgradeManager::getRange(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) return up->baseRange * up->rangeMultiplier;
    return 192.f;
}

int UpgradeManager::getRank(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) return up->rank;
    return 0;
}

int UpgradeManager::getLevel(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) return up->level;
    return 0;
}

bool UpgradeManager::isStatAtLimit(const std::string& towerType, const std::string& statKey) const {
    const auto* up = getUpgrade(towerType);
    if (!up) return true;

    int currentStatLvl = 0;
    if (statKey == "damage") currentStatLvl = up->damageLvl;
    else if (statKey == "firerate") currentStatLvl = up->firerateLvl;
    else if (statKey == "range") currentStatLvl = up->rangeLvl;
    else if (statKey == "rank") return up->rank >= (int)MAX_TOWER_RANK;
    else if (statKey == "level") return up->level >= (int)MAX_INGAME_LEVEL;

    return currentStatLvl >= getMaxStatLevel(towerType);
}

int UpgradeManager::getMaxStatLevel(const std::string& towerType) const {
    const auto* up = getUpgrade(towerType);
    if (!up) return 0;
    return up->rank * (int)UPGRADES_PER_RANK;
}

int UpgradeManager::getUpgradeCost(const std::string& towerType, int statIndex) const {
    const auto* up = getUpgrade(towerType);
    if (!up) return 100;

    int baseCost = 0;
    int currentLvl = 0;

    if (statIndex == 0) { // Rank
        baseCost = up->costRank;
        currentLvl = up->rank;
    }
    else if (statIndex == 1) { // Damage
        baseCost = up->costDamage;
        currentLvl = up->damageLvl;
    }
    else if (statIndex == 2) { // Firerate
        baseCost = up->costFirerate;
        currentLvl = up->firerateLvl;
    }
    else if (statIndex == 3) { // Range
        baseCost = up->costRange;
        currentLvl = up->rangeLvl;
    }
    else if (statIndex == 4) { // Max Level
        baseCost = up->costLevel;
        currentLvl = up->level;
    }

    float price = baseCost * (1.0f + currentLvl * 0.5f);
    return (int)price;
}

int UpgradeManager::getUpgradeCost(const std::string& towerType, const std::string& statKey) const {
    static const std::unordered_map<std::string, int> keyToIndex = {
        {"rank", 0},
        {"damage", 1},
        {"firerate", 2},
        {"range", 3},
        {"level", 4}
    };
    auto it = keyToIndex.find(statKey);
    if (it != keyToIndex.end()) return getUpgradeCost(towerType, it->second);
    return 100;
}

int UpgradeManager::getRandomMoney(float multiplier) const {
    int base = Math::Random::getInt(moneyMin, moneyMax);
    return static_cast<int>(base * multiplier);
}

void UpgradeManager::upgradeDamage(const std::string& towerType, float increment) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType) {
            if (up.damageLvl < getMaxStatLevel(towerType)) {
                up.damageLvl++;
                up.damageMultiplier += increment;
                if (onUpgradeChanged) onUpgradeChanged();
            }
            break;
        }
    }
}

void UpgradeManager::upgradeFirerate(const std::string& towerType, float increment) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType) {
            if (up.firerateLvl < getMaxStatLevel(towerType)) {
                up.firerateLvl++;
                up.firerateMultiplier += increment;
                if (onUpgradeChanged) onUpgradeChanged();
            }
            break;
        }
    }
}

void UpgradeManager::upgradeRange(const std::string& towerType, float increment) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType) {
            if (up.rangeLvl < getMaxStatLevel(towerType)) {
                up.rangeLvl++;
                up.rangeMultiplier += increment;
                if (onUpgradeChanged) onUpgradeChanged();
            }
            break;
        }
    }
}

void UpgradeManager::upgradeRank(const std::string& towerType) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType && up.rank < (int)MAX_TOWER_RANK) {
            up.rank++;
            up.damageMultiplier += 0.02f;
            up.firerateMultiplier += 0.02f;
            up.rangeMultiplier += 0.02f;
            if (onUpgradeChanged) onUpgradeChanged();
            break;
        }
    }
}

void UpgradeManager::upgradeMaxLevel(const std::string& towerType) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType && up.level < (int)MAX_INGAME_LEVEL) {
            up.level++;
            if (onUpgradeChanged) onUpgradeChanged();
            break;
        }
    }
}
