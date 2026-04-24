#include "SaveManager.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <iostream>

#ifdef __ANDROID__
#include <SFML/System/NativeActivity.hpp>
#include <android/native_activity.h>
#endif

SaveManager::SaveManager() {
    savePath = getSavePath();
    setDefaults(); // Сначала ставим дефолты на случай отсутствия файла
    load();
}

std::string SaveManager::getSavePath() {
#ifdef __ANDROID__
    ANativeActivity* activity = sf::getNativeActivity();
    // Пишем во внутреннюю защищенную папку приложения
    return std::string(activity->internalDataPath) + "/progress.json";
#else
    // На ПК сохраняем в папку с конфигами
    return "data/config/progress.json";
#endif
}

void SaveManager::setDefaults() {
    money = 0;
    levels.clear();
    // Открываем первый уровень по умолчанию
    levels["level01"] = { 0, true }; // 0 звезд, разблокирован
    towerDataBlob = json::object(); // Пустой объект
}

void SaveManager::load() {
    std::ifstream file(savePath);
    if (!file.is_open()) {
        LOGI("Save file not found at %s. Using defaults.", savePath.c_str());
        return;
    }

    try {
        json j;
        file >> j;

        // Загружаем деньги
        money = j.value("money", 0);

        // Загружаем уровни
        if (j.contains("levels")) {
            levels = j["levels"].get<std::map<std::string, LevelProgress>>();
        }

        // Загружаем "черный ящик" данных башен
        if (j.contains("tower_data")) {
            towerDataBlob = j["tower_data"];
        }

        LOGI("Progress successfully loaded from %s", savePath.c_str());
    } catch (const std::exception& e) {
        LOGE("Failed to parse progress.json: %s", e.what());
        setDefaults(); // Если файл битый, откатываемся к началу
    }
}

void SaveManager::save() {
    try {
        json j;
        j["money"] = money;
        j["levels"] = levels;
        j["tower_data"] = towerDataBlob;

        std::ofstream file(savePath);
        if (file.is_open()) {
            file << j.dump(4); // Сохраняем с отступом 4 пробела
            LOGI("Progress saved to %s", savePath.c_str());
        } else {
            LOGE("Could not open %s for writing!", savePath.c_str());
        }
    } catch (const std::exception& e) {
        LOGE("Error during saving progress: %s", e.what());
    }
}

// --- Реализация логики ---

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
    
    if (it != levels.end()) {
        return it->second.unlocked;
    }
    
    return false; 
}

void SaveManager::unlockLevel(const std::string& levelId) {
    // Устанавливаем флаг разблокировки. 
    // Если записи об уровне ещё нет, она будет создана автоматически.
    levels[levelId].unlocked = true;
    // Мы не вызываем здесь save() принудительно, чтобы можно было 
    // изменить несколько настроек сразу, а потом сохранить один раз.
}