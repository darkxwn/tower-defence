#include "SaveManager.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <iostream>

#ifdef __ANDROID__
#include <SFML/System/NativeActivity.hpp>
#include <android/native_activity.h>
#endif

using Engine::Logger;

SaveManager::SaveManager() {
    savePath = getSavePath();
    setDefaults(); 
    load();
}

std::string SaveManager::getSavePath() {
#ifdef __ANDROID__
    ANativeActivity* activity = sf::getNativeActivity();
    return std::string(activity->internalDataPath) + "/progress.json";
#else
    return "data/config/progress.json";
#endif
}

void SaveManager::setDefaults() {
    money = 0;
    moneyMultiplier = 1.0f;
    levels.clear();
    levels["level01"] = { 0, true };
    towerDataBlob = json::object();
}

void SaveManager::load() {
    std::ifstream file(savePath);
    if (!file.is_open()) {
        Logger::debug("Save file not found at {}. Using defaults.", savePath.c_str());
        return;
    }

    try {
        json j;
        file >> j;

        money = j.value("money", 0);
        moneyMultiplier = j.value("money_multiplier", 1.0f);

        if (j.contains("levels")) {
            levels = j["levels"].get<std::map<std::string, LevelProgress>>();
        }

        if (j.contains("towers")) {
            towerDataBlob = j["towers"];
        }

        Logger::debug("Прогресс успешно загружен с {}", savePath);
    } catch (const std::exception& e) {
        Logger::error("Ошибка парсинга progress.json: {}", e.what());
        setDefaults();
    }
}

void SaveManager::save() {
    try {
        json j;
        j["money"] = money;
        j["money_multiplier"] = moneyMultiplier;
        j["levels"] = levels;
        j["towers"] = towerDataBlob;

        std::ofstream file(savePath);
        if (file.is_open()) {
            file << j.dump(4);
            Logger::debug("Progress saved to {}", savePath);
        } else {
            Logger::error("Could not open {} for writing!", savePath);
        }
    } catch (const std::exception& e) {
        Logger::error("Error during saving progress: {}", e.what());
    }
}

int SaveManager::getMoney() const { return money; }

void SaveManager::addMoney(int amount) {
    money += amount;
}

bool SaveManager::spendMoney(int amount) {
    if (money >= amount) {
        money -= amount;
        return true;
    }
    return false;
}

float SaveManager::getMoneyMultiplier() const { return moneyMultiplier; }

void SaveManager::setMoneyMultiplier(float multiplier) { moneyMultiplier = multiplier; }

int SaveManager::getStars(const std::string& levelId) const {
    if (levels.count(levelId)) return levels.at(levelId).stars;
    return 0;
}

void SaveManager::setStars(const std::string& levelId, int stars) {
    if (levels[levelId].stars < stars) {
        levels[levelId].stars = stars;
    }
}

bool SaveManager::isUnlocked(const std::string& levelId) const {
    auto it = levels.find(levelId);
    if (it != levels.end()) return it->second.unlocked;
    return false; 
}

void SaveManager::unlockLevel(const std::string& levelId) {
    levels[levelId].unlocked = true;
}
