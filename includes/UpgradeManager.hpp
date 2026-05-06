#pragma once
#include <string>
#include <vector>
#include <functional>
#include <random>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС UPGRADEMANAGER
//
///////////////////////////////////////////////////////////////////////////

class UpgradeManager {
public:
    static constexpr float RANK_BONUS = 0.02f;
    static const unsigned int MAX_TOWER_RANK = 5;
    static const unsigned int MAX_INGAME_LEVEL = 5;
    static const unsigned int UPGRADES_PER_RANK = 3; // Сколько уровней стата дает 1 ранг

    static int moneyMin;
    static int moneyMax;

    struct Upgrade {
        unsigned int level = 0;
        unsigned int maxLevel = 0;
        float value = 0;
        float baseValue = 0;
        int cost = 0;
    };

    struct TowerUpgrade {
        std::string towerType;
        Upgrade level;
        Upgrade rank;
        Upgrade damage;
        Upgrade firerate;
        Upgrade range;
    };

    struct MetaUpgrade {
        std::string id;
        Upgrade upgrade;
    };

private:
    std::vector<TowerUpgrade> upgrades;
    std::vector<MetaUpgrade> metaUpgrades;
    std::function<void()> onUpgradeChanged;

public:
    void setSaveCallback(std::function<void()> callback);
    void initDefaults();

    const std::vector<TowerUpgrade>& getAllUpgrades() const;
    void setAllUpgrades(const std::vector<TowerUpgrade>& data);

    const std::vector<MetaUpgrade>& getAllMetaUpgrades() const;
    void setAllMetaUpgrades(const std::vector<MetaUpgrade>& data);

    const TowerUpgrade* getUpgrade(const std::string& towerType) const;
    const MetaUpgrade* getMetaUpgrade(const std::string& id) const;

    // Геттеры статов
    float getDamage(const std::string& towerType) const;
    float getFirerate(const std::string& towerType) const;
    float getRange(const std::string& towerType) const;
    int getRank(const std::string& towerType) const;
    int getLevel(const std::string& towerType) const;

    // Проверка лимитов
    bool isStatAtLimit(const std::string& towerType, const std::string& statKey) const;
    int getMaxStatLevel(const std::string& towerType) const;

    // Цены
    int getUpgradeCost(const std::string& towerType, int statIndex) const;
    int getUpgradeCost(const std::string& towerType, const std::string& statKey) const;

    // Деньги
    int getRandomMoney(float multiplier = 1.0f) const;

    // Методы улучшения
    void upgradeDamage(const std::string& towerType, float increment);
    void upgradeFirerate(const std::string& towerType, float increment);
    void upgradeRange(const std::string& towerType, float increment);
    void upgradeRank(const std::string& towerType);
    void upgradeMaxLevel(const std::string& towerType);

    void upgradeMeta(const std::string& id);
    int getMetaUpgradeCost(const std::string& id) const;

    // Получение глобальных бонусов
    float getGlobalMoneyMultiplier() const;
    unsigned int getGlobalCoinsBonus() const;
    unsigned int getGlobalBaseHpBonus() const;
};

// Сериализация
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpgradeManager::Upgrade, level, maxLevel, value, baseValue, cost)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpgradeManager::TowerUpgrade, towerType, level, rank, damage, firerate, range)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpgradeManager::MetaUpgrade, id, upgrade)

