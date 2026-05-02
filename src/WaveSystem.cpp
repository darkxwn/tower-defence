#include "WaveSystem.hpp"
#include "GameData.hpp"
#include "utils/FileReader.hpp"
#include "Enemy.hpp"
#include "utils/Math.hpp"
#include <fstream>
#include <sstream>
#include <cmath>

void WaveSystem::init(const std::vector<std::string>& allowedEnemies) {
    mapAllowedEnemies = allowedEnemies;
    // Если список пуст, используем всех доступных врагов из GameData
    if (mapAllowedEnemies.empty()) {
        mapAllowedEnemies = GameData::getEnemyTypes();
    }
    
    currentWave = 0;
    state = WaveState::Idle;
    wavesUntilTypeChange = 0;
}

// Обновление системы волн
void WaveSystem::update(float deltaTime, std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<sf::Vector2i>& path) {
    if (state == WaveState::Idle) return;

    // Пауза между волнами
    if (state == WaveState::Waiting) {
        waitTimer -= deltaTime;
        if (waitTimer <= 0.f)
            startWave();
        return;
    }

    // Ожидание уничтожения всех врагов текущей волны
    if (state == WaveState::Fighting) {
        if (enemies.empty()) {
            state = WaveState::Waiting;
            waitTimer = waitInterval;
        }
        return;
    }

    // Постепенный спавн врагов
    if (state == WaveState::Spawning) {
        auto stats = GameData::getEnemy(currentEnemyType);

        spawnTimer += deltaTime;
        if (spawnTimer < stats.spawnInterval) return;
        spawnTimer = 0.f;

        // Экспоненциальное масштабирование HP (броня не масштабируется!)
        int scaledHp = static_cast<int>(stats.health * std::pow(1.06f, currentWave - 1));
        
        enemies.push_back(std::make_unique<Enemy>(
            currentEnemyType, 
            scaledHp, 
            stats.speed, 
            stats.reward, 
            stats.points, 
            stats.armor, 
            path
        ));

        spawnedCount++;

        // Если заспавнили всех, переходим в режим боя
        if (spawnedCount >= totalInWave) {
            spawnedCount = 0;
            state = WaveState::Fighting;
        }
    }
}

// Запуск волны
void WaveSystem::startWave() {
    if (state != WaveState::Idle && state != WaveState::Waiting) return;
    
    currentWave++;
    
    // 1. Смена типа врага
    if (currentWave <= 10) {
        auto it = std::find(mapAllowedEnemies.begin(), mapAllowedEnemies.end(), "basic");
        if (it != mapAllowedEnemies.end()) {
            currentEnemyType = "basic";
        } else if (!mapAllowedEnemies.empty()) {
            currentEnemyType = mapAllowedEnemies[0];
        }
        wavesUntilTypeChange = 0; 
    }
    else if (wavesUntilTypeChange <= 0) {
        if (!mapAllowedEnemies.empty()) {
            int idx = Math::Random::getInt(0, (int)mapAllowedEnemies.size() - 1);
            currentEnemyType = mapAllowedEnemies[idx];
            wavesUntilTypeChange = Math::Random::getInt(2, 4); // Держим тип от 2 до 4 волн
        }
    }
    wavesUntilTypeChange--;

    // 2. Расчет количества
    totalInWave = (8 + (currentWave / 3) * 5) + Math::Random::getInt(-3, 3);
    if (totalInWave < 1) totalInWave = 1;
    
    spawnedCount = 0;
    state = WaveState::Spawning;
    spawnTimer = 100.f; // Чтобы первый враг спавнился сразу
}

// Проверка завершения (бесконечный режим)
bool WaveSystem::isFinished() const {
    return false;
}

// Получение текущего состояния
WaveState WaveSystem::getState() const {
    return state;
}

// Получение таймера паузы
float WaveSystem::getWaitTimer() const {
    return waitTimer;
}

// Получение индекса текущей волны
int WaveSystem::getCurrentWave() const {
    return currentWave;
}
