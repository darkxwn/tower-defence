#include "UpgradeManager.hpp"

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС UPGRADEMANAGER
//
///////////////////////////////////////////////////////////////////////////

// Установка callback для сохранения при изменении улучшения
void UpgradeManager::setSaveCallback(std::function<void(const TowerUpgrade&)> callback) {
    onUpgradeChanged = callback;
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