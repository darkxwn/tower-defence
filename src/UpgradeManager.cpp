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
int UpgradeManager::moneyMax = 5;

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

        up.level.level = stats.level;
        up.level.maxLevel = MAX_INGAME_LEVEL;
        up.level.value = (float)stats.level;
        up.level.baseValue = (float)stats.level;
        up.level.cost = stats.costLevel;

        up.rank.level = stats.rank;
        up.rank.maxLevel = MAX_TOWER_RANK;
        up.rank.value = (float)stats.rank;
        up.rank.baseValue = (float)stats.rank;
        up.rank.cost = stats.costRank;

        up.damage.level = 0;
        up.damage.maxLevel = up.rank.level * UPGRADES_PER_RANK;
        up.damage.value = 1.0f; // multiplier
        up.damage.baseValue = (float)stats.damage;
        up.damage.cost = stats.costDamage;

        up.firerate.level = 0;
        up.firerate.maxLevel = up.rank.level * UPGRADES_PER_RANK;
        up.firerate.value = 1.0f; // multiplier
        up.firerate.baseValue = stats.firerate;
        up.firerate.cost = stats.costFirerate;

        up.range.level = 0;
        up.range.maxLevel = up.rank.level * UPGRADES_PER_RANK;
        up.range.value = 1.0f; // multiplier
        up.range.baseValue = stats.range;
        up.range.cost = stats.costRange;

        upgrades.push_back(up);
    }

    metaUpgrades.clear();
    metaUpgrades.push_back({"globalCoins",  {0, 10, 0.f, 0.f, 500}});
    metaUpgrades.push_back({"globalMoney",  {0, 10, 1.0f, 1.0f, 750}});
    metaUpgrades.push_back({"globalBaseHp", {0, 10, 0.f, 0.f, 400}});
}

const std::vector<UpgradeManager::TowerUpgrade>& UpgradeManager::getAllUpgrades() const {
    return upgrades;
}

void UpgradeManager::setAllUpgrades(const std::vector<TowerUpgrade>& data) {
    upgrades = data;
}

const std::vector<UpgradeManager::MetaUpgrade>& UpgradeManager::getAllMetaUpgrades() const {
    return metaUpgrades;
}

void UpgradeManager::setAllMetaUpgrades(const std::vector<MetaUpgrade>& data) {
    metaUpgrades = data;
}

const UpgradeManager::TowerUpgrade* UpgradeManager::getUpgrade(const std::string& towerType) const {
    for (const auto& up : upgrades) {
        if (up.towerType == towerType) return &up;
    }
    return nullptr;
}

const UpgradeManager::MetaUpgrade* UpgradeManager::getMetaUpgrade(const std::string& id) const {
    for (const auto& up : metaUpgrades) {
        if (up.id == id) return &up;
    }
    return nullptr;
}

float UpgradeManager::getDamage(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) return up->damage.baseValue * up->damage.value;
    return 35.f;
}

float UpgradeManager::getFirerate(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) return up->firerate.baseValue * up->firerate.value;
    return 1.0f;
}

float UpgradeManager::getRange(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) return up->range.baseValue * up->range.value;
    return 192.f;
}

int UpgradeManager::getRank(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) return up->rank.level;
    return 0;
}

int UpgradeManager::getLevel(const std::string& towerType) const {
    if (const auto* up = getUpgrade(towerType)) return up->level.level;
    return 0;
}

bool UpgradeManager::isStatAtLimit(const std::string& towerType, const std::string& statKey) const {
    const auto* up = getUpgrade(towerType);
    if (!up) return true;

    if (statKey == "damage") return up->damage.level >= up->damage.maxLevel;
    else if (statKey == "firerate") return up->firerate.level >= up->firerate.maxLevel;
    else if (statKey == "range") return up->range.level >= up->range.maxLevel;
    else if (statKey == "rank") return up->rank.level >= up->rank.maxLevel;
    else if (statKey == "level") return up->level.level >= up->level.maxLevel;

    return true;
}

int UpgradeManager::getMaxStatLevel(const std::string& towerType) const {
    const auto* up = getUpgrade(towerType);
    if (!up) return 0;
    return up->damage.maxLevel;
}

int UpgradeManager::getUpgradeCost(const std::string& towerType, int statIndex) const {
    const auto* up = getUpgrade(towerType);
    if (!up) return 100;

    int baseCost = 0;
    int currentLvl = 0;

    if (statIndex == 0) { // Rank
        baseCost = up->rank.cost;
        currentLvl = up->rank.level;
    }
    else if (statIndex == 1) { // Damage
        baseCost = up->damage.cost;
        currentLvl = up->damage.level;
    }
    else if (statIndex == 2) { // Firerate
        baseCost = up->firerate.cost;
        currentLvl = up->firerate.level;
    }
    else if (statIndex == 3) { // Range
        baseCost = up->range.cost;
        currentLvl = up->range.level;
    }
    else if (statIndex == 4) { // Max Level
        baseCost = up->level.cost;
        currentLvl = up->level.level;
    }

    float price = (float)baseCost * (1.0f + (float)currentLvl * 0.3f);
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

int UpgradeManager::getMetaUpgradeCost(const std::string& id) const {
    const auto* up = getMetaUpgrade(id);
    if (!up) return 0;
    return (int)((float)(up->upgrade.level + 1) * (float)up->upgrade.cost);
}

int UpgradeManager::getRandomMoney(float multiplier) const {
    int base = Math::Random::getInt(moneyMin, moneyMax);
    return static_cast<int>((float)base * multiplier);
}

void UpgradeManager::upgradeDamage(const std::string& towerType, float increment) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType) {
            if (up.damage.level < up.damage.maxLevel) {
                up.damage.level++;
                up.damage.value += increment;
                if (onUpgradeChanged) onUpgradeChanged();
            }
            break;
        }
    }
}

void UpgradeManager::upgradeFirerate(const std::string& towerType, float increment) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType) {
            if (up.firerate.level < up.firerate.maxLevel) {
                up.firerate.level++;
                up.firerate.value += increment;
                if (onUpgradeChanged) onUpgradeChanged();
            }
            break;
        }
    }
}

void UpgradeManager::upgradeRange(const std::string& towerType, float increment) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType) {
            if (up.range.level < up.range.maxLevel) {
                up.range.level++;
                up.range.value += increment;
                if (onUpgradeChanged) onUpgradeChanged();
            }
            break;
        }
    }
}

void UpgradeManager::upgradeRank(const std::string& towerType) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType && up.rank.level < up.rank.maxLevel) {
            up.rank.level++;
            
            // Увеличиваем капы для других статов
            up.damage.maxLevel = up.rank.level * UPGRADES_PER_RANK;
            up.firerate.maxLevel = up.rank.level * UPGRADES_PER_RANK;
            up.range.maxLevel = up.rank.level * UPGRADES_PER_RANK;

            const float RANK_BONUS = 0.01f;
            up.damage.value += RANK_BONUS;
            up.firerate.value += RANK_BONUS;
            up.range.value += RANK_BONUS;

            if (onUpgradeChanged) onUpgradeChanged();
            break;
        }
    }
}

void UpgradeManager::upgradeMaxLevel(const std::string& towerType) {
    for (auto& up : upgrades) {
        if (up.towerType == towerType && up.level.level < up.level.maxLevel) {
            up.level.level++;
            if (onUpgradeChanged) onUpgradeChanged();
            break;
        }
    }
}

void UpgradeManager::upgradeMeta(const std::string& id) {
    for (auto& up : metaUpgrades) {
        if (up.id == id && up.upgrade.level < up.upgrade.maxLevel) {
            up.upgrade.level++;
            if (id == "globalMoney") {
                up.upgrade.value += 0.1f;
            } else {
                up.upgrade.value += 1.f;
            }
            if (onUpgradeChanged) onUpgradeChanged();
            break;
        }
    }
}

float UpgradeManager::getGlobalMoneyMultiplier() const {
    if (const auto* up = getMetaUpgrade("globalMoney")) return up->upgrade.value;
    return 1.0f;
}

unsigned int UpgradeManager::getGlobalCoinsBonus() const {
    if (const auto* up = getMetaUpgrade("globalCoins")) return (unsigned int)up->upgrade.value;
    return 0;
}

unsigned int UpgradeManager::getGlobalBaseHpBonus() const {
    if (const auto* up = getMetaUpgrade("globalBaseHp")) return (unsigned int)up->upgrade.value;
    return 0;
}
