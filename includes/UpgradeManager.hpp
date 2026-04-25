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
    static const unsigned int MAX_TOWER_LEVEL = 10;
    static int moneyMin;
    static int moneyMax;

    struct TowerUpgrade {
        std::string towerType;
        float baseDamage = 35.f;
        float baseFirerate = 1.0f;
        float baseRange = 192.f;
        int level = 0;
        float damageMultiplier = 1.0f;
        float firerateMultiplier = 1.0f;
        float rangeMultiplier = 1.0f;
        float moneyMultiplier = 1.0f;
        int costDamage = 50;
        int costFirerate = 80;
        int costRange = 100;
        int costLevel = 200;
    };

private:
    std::vector<TowerUpgrade> upgrades;
    std::function<void(const TowerUpgrade&)> onUpgradeChanged;

public:
    // Установка callback для сохранения при изменении улучшения
    void setSaveCallback(std::function<void(const TowerUpgrade&)> callback);

    // Инициализация базовыми значениями из GameData
    void initDefaults();

    // Получение всех улучшений (для SaveManager)
    const std::vector<TowerUpgrade>& getAllUpgrades() const;

    // Установка всех улучшений (от SaveManager)
    void setAllUpgrades(const std::vector<TowerUpgrade>& data);

    // Получение улучшения для типа башни
    const TowerUpgrade* getUpgrade(const std::string& towerType) const;

    // Получение актуального урона (база * множитель)
    float getDamage(const std::string& towerType) const;

    // Получение актуальной скорости (база * множитель)
    float getFirerate(const std::string& towerType) const;

    // Получение актуальной дальности (база * множитель)
    float getRange(const std::string& towerType) const;

    // Получение уровня башни
    int getLevel(const std::string& towerType) const;

    // Получение множителя денег
    float getMoneyMultiplier(const std::string& towerType) const;

    // Получение текущей стоимости улучшения (statIndex: 0=damage, 1=firerate, 2=range, 3=level)
    int getUpgradeCost(const std::string& towerType, int statIndex) const;

    // Получение текущей стоимости улучшения по ключу
    int getUpgradeCost(const std::string& towerType, const std::string& statKey) const;

    // Получение случайного количества денег за убийство
    int getRandomMoney() const;

    // Изменение улучшения урона
    void upgradeDamage(const std::string& towerType, float increment);

    // Изменение улучшения скорости
    void upgradeFirerate(const std::string& towerType, float increment);

    // Изменение улучшения дальности
    void upgradeRange(const std::string& towerType, float increment);

    // Изменение уровня башни
    void upgradeLevel(const std::string& towerType);
};

// Сериализация для SaveManager (NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpgradeManager::TowerUpgrade,
    towerType, baseDamage, baseFirerate, baseRange, level,
    damageMultiplier, firerateMultiplier, rangeMultiplier, moneyMultiplier,
    costDamage, costFirerate, costRange, costLevel)