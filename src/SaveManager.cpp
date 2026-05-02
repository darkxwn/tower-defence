#include "SaveManager.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

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
    globalCoinsLvl = 0;
    globalMoneyLvl = 0;
    globalBaseHpLvl = 0;
    levels.clear();
    // Первый уровень всегда открыт
    levels["level01"] = { 0, 0, 0, true };
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

        // Anti-Tamper Check
        if constexpr (ENABLE_ANTI_TAMPER) {
            size_t savedHash = j.value("hashChecksum", 0ULL);
            if (savedHash != 0ULL) {
                json checkJ = j;
                checkJ.erase("hashChecksum");
                std::string dataDump = checkJ.dump();
                size_t calculatedHash = std::hash<std::string>{}(dataDump + "GyurzaSecretSalt");

                if (savedHash != calculatedHash) {
                    Logger::error("Anti-Tamper: Файл сохранения изменен вручную! Сброс прогресса.");
                    setDefaults();
                    return;
                }
            }
        }

        // Читаем в camelCase согласно PLAN.md
        money = j.value("money", 0);
        globalCoinsLvl = j.value("globalCoinsLvl", 0);
        globalMoneyLvl = j.value("globalMoneyLvl", 0);
        globalBaseHpLvl = j.value("globalBaseHpLvl", 0);

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
        // Пишем в camelCase согласно PLAN.md
        j["money"] = money;
        j["globalCoinsLvl"] = globalCoinsLvl;
        j["globalMoneyLvl"] = globalMoneyLvl;
        j["globalBaseHpLvl"] = globalBaseHpLvl;
        j["levels"] = levels;
        j["towers"] = towerDataBlob;
        if constexpr (ENABLE_ANTI_TAMPER) {
            // Anti-Tamper Hash
            std::string dataDump = j.dump();
            size_t hashChecksum = std::hash<std::string>{}(dataDump + "GyurzaSecretSalt_2025");
            j["hashChecksum"] = hashChecksum;
        }

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

float SaveManager::getMoneyMultiplier() const { 
    return 1.0f + (globalMoneyLvl * 0.1f); 
}

int SaveManager::getStars(const std::string& levelId) const {
    if (levels.count(levelId)) return levels.at(levelId).stars;
    return 0;
}

int SaveManager::getBestScore(const std::string& levelId) const {
    if (levels.count(levelId)) return levels.at(levelId).bestScore;
    return 0;
}

int SaveManager::getMaxWave(const std::string& levelId) const {
    if (levels.count(levelId)) return levels.at(levelId).maxWave;
    return 0;
}

void SaveManager::updateLevelRecord(const std::string& levelId, int stars, int score, int wave) {
    auto& p = levels[levelId];
    p.stars = std::max(p.stars, stars);
    p.bestScore = std::max(p.bestScore, score);
    p.maxWave = std::max(p.maxWave, wave);
}

bool SaveManager::isUnlocked(const std::string& levelId) const {
    auto it = levels.find(levelId);
    if (it != levels.end()) return it->second.unlocked;
    return false; 
}

void SaveManager::unlockLevel(const std::string& levelId) {
    levels[levelId].unlocked = true;
}
