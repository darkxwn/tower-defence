#pragma once
#include <string>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС UPGRADEMANAGER
//
///////////////////////////////////////////////////////////////////////////

class UpgradeManager {
public:
    struct TowerUpgrade {
        std::string towerType;
        float baseDamage = 35.f;
        float baseFirerate = 1.0f;
        float baseRange = 192.f;
        float damageMultiplier = 1.0f;
        float firerateMultiplier = 1.0f;
        float rangeMultiplier = 1.0f;
    };

private:
    std::vector<TowerUpgrade> upgrades;
    std::function<void(const TowerUpgrade&)> onUpgradeChanged;

public:
    // Установка callback для сохранения при изменении улучшения
    void setSaveCallback(std::function<void(const TowerUpgrade&)> callback);

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

    // Изменение улучшения урона
    void upgradeDamage(const std::string& towerType, float increment);

    // Изменение улучшения скорости
    void upgradeFirerate(const std::string& towerType, float increment);

    // Изменение улучшения дальности
    void upgradeRange(const std::string& towerType, float increment);
};

// Сериализация для SaveManager (NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UpgradeManager::TowerUpgrade,
    towerType, baseDamage, baseFirerate, baseRange,
    damageMultiplier, firerateMultiplier, rangeMultiplier)