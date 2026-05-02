#pragma once
#include "Enemy.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <list>
#include <memory>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС WAVESYSTEM
//
///////////////////////////////////////////////////////////////////////////

// Состояния системы волн
enum class WaveState {
    Idle, // ожидание запуска первой волны
    Waiting, // пауза между волнами
    Spawning, // спавн врагов
    Fighting, // все заспавнены, ожидание завершения
    Finished // все волны завершены (не используется в бесконечном режиме)
};

class WaveSystem {
private:
    int currentWave = 0; // индекс текущей волны
    int spawnedCount = 0; // сколько заспавнено в текущей волне
    int totalInWave = 0; // сколько всего в текущей волне

    float spawnTimer = 0.f; // таймер спавна
    float waitTimer = 0.f; // таймер паузы
    float waitInterval = 8.f; // пауза между волнами

    WaveState state = WaveState::Idle; // текущее состояние

    // Данные для процедурной генерации
    std::string currentEnemyType;
    int wavesUntilTypeChange = 0;
    std::vector<std::string> mapAllowedEnemies;

public:
    // Инициализация системы под карту (список разрешенных врагов)
    void init(const std::vector<std::string>& allowedEnemies);

    // Обновление системы волн
    void update(float deltaTime, std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<sf::Vector2i>& path);

    // Запуск волны
    void startWave();

    // Проверка завершения (в бесконечном режиме всегда false)
    bool isFinished() const;

    // Получение текущего состояния
    WaveState getState() const;

    // Получение таймера паузы
    float getWaitTimer() const;

    // Получение индекса текущей волны
    int getCurrentWave() const;
    
    // Метод заглушка для обратной совместимости (если нужно)
    void loadWaves(const std::string& path) {}
};
