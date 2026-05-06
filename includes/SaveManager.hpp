#pragma once
#include <string>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Информация о прогрессе конкретного уровня
struct LevelProgress {
    int stars = 0;
    int bestScore = 0;
    int maxWave = 0;
    bool unlocked = false;
};

// Чтобы nlohmann::json умел работать со структурой LevelProgress автоматически
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LevelProgress, stars, bestScore, maxWave, unlocked)

class SaveManager {
private:
    static constexpr bool ENABLE_ANTI_TAMPER = false;

    std::string savePath;
    
    // Глобальная валюта (мета)
    int money = 0;
    
    std::map<std::string, LevelProgress> levels;

    // Универсальный контейнер для данных улучшений (башен и меты)
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

    // Прогресс уровней
    int getStars(const std::string& levelId) const;
    int getBestScore(const std::string& levelId) const;
    int getMaxWave(const std::string& levelId) const;
    
    // Обновление рекордов (сохранит только если новые значения лучше)
    void updateLevelRecord(const std::string& levelId, int stars, int score, int wave);

    bool isUnlocked(const std::string& levelId) const;
    void unlockLevel(const std::string& levelId);

    // Универсальная работа с данными улучшений
    template <typename T>
    void setUpgradeData(const T& dataObject) {
        towerDataBlob = dataObject;
    }

    template <typename T>
    bool getUpgradeData(T& outDataObject) const {
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
