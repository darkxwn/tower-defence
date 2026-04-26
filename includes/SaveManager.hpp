#pragma once
#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Информация о прогрессе конкретного уровня
struct LevelProgress {
    int stars = 0;
    bool unlocked = false;
};

// Чтобы nlohmann::json умел работать со структурой LevelProgress автоматически
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LevelProgress, stars, unlocked)

class SaveManager {
private:
    std::string savePath;
    
    // Основные данные
    int money = 0;
    float moneyMultiplier = 1.0f;
    std::map<std::string, LevelProgress> levels;

    // Универсальный контейнер для данных башен (хранит любой JSON)
    json towerDataBlob; 

    void setDefaults();
    std::string getSavePath();

public:
    SaveManager();

    void load();
    void save();

    // Глобальные деньги
    int getMoney() const;
    void addMoney(int amount);
    bool spendMoney(int amount);
    float getMoneyMultiplier() const;
    void setMoneyMultiplier(float multiplier);

    // Прогресс уровней
    int getStars(const std::string& levelId) const;
    void setStars(const std::string& levelId, int stars);
    bool isUnlocked(const std::string& levelId) const;
    void unlockLevel(const std::string& levelId);

    // Универсальная работа с данными башен
    template <typename T>
    void setTowerData(const T& dataObject) {
        towerDataBlob = dataObject;
    }

    template <typename T>
    bool getTowerData(T& outDataObject) const {
        if (towerDataBlob.is_null() || towerDataBlob.empty()) {
            return false;
        }
        try {
            outDataObject = towerDataBlob.get<T>();
            return true;
        } catch (...) {
            return false;
        }
    }
};
