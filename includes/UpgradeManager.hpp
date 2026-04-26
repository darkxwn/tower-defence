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
    static const unsigned int MAX_TOWER_RANK = 10;
    static const unsigned int MAX_INGAME_LEVEL = 10;
    static const unsigned int UPGRADES_PER_RANK = 5; // Сколько уровней стата дает 1 ранг

    static int moneyMin;
    static int moneyMax;

    struct TowerUpgrade {
        std::string towerType;
        
        // Базовые статы из конфига
        float baseDamage = 35.f;
        float baseFirerate = 1.0f;
        float baseRange = 192.f;

        // Глобальный прогресс
        int rank = 0;
        int level = 0; // Максимально доступный уровень в игре

        // Уровни прокачки конкретных характеристик (шаги)
        int damageLvl = 0;
        int firerateLvl = 0;
        int rangeLvl = 0;

        // Итоговые множители (от глобальной прокачки)
        float damageMultiplier = 1.0f;
        float firerateMultiplier = 1.0f;
        float rangeMultiplier = 1.0f;

        // Цены (базовые)
        int costRank = 200;
        int costDamage = 50;
        int costFirerate = 80;
        int costRange = 100;
        int costLevel = 250;
    };

private:
    std::vector<TowerUpgrade> upgrades;
    std::function<void()> onUpgradeChanged;

public:
    void setSaveCallback(std::function<void()> callback);
    void initDefaults();

    const std::vector<TowerUpgrade>& getAllUpgrades() const;
    void setAllUpgrades(const std::vector<TowerUpgrade>& data);

    const TowerUpgrade* getUpgrade(const std::string& towerType) const;

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
};

// Сериализация
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpgradeManager::TowerUpgrade,
    towerType, baseDamage, baseFirerate, baseRange, rank, level,
    damageLvl, firerateLvl, rangeLvl,
    damageMultiplier, firerateMultiplier, rangeMultiplier,
    costRank, costDamage, costFirerate, costRange, costLevel)
