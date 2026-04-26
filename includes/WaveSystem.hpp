#pragma once
#include "Enemy.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <list>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС WAVESYSTEM
//
///////////////////////////////////////////////////////////////////////////

// Структура волны
struct Wave {
    std::string type; // идентификатор типа врага (строка)
    int count; // количество врагов в группе
};

// Состояния системы волн
enum class WaveState {
    Idle, // ожидание запуска первой волны
    Waiting, // пауза между волнами
    Spawning, // спавн врагов
    Fighting, // все заспавнены, ожидание завершения
    Finished // все волны завершены
};

class WaveSystem {
private:
    std::vector<Wave> waves; // список всех волн

    int currentWave = 0; // индекс текущей волны
    int spawned = 0; // количество заспавненных врагов

    float spawnTimer = 0.f; // таймер спавна
    float waitTimer = 0.f; // таймер паузы
    float waitInterval = 8.f; // пауза между волнами

    WaveState state = WaveState::Idle; // текущее состояние

    // Интервал спавна по типу врага
    float getSpawnInterval(const std::string& type) const;

public:
    // Загрузка волн из файла
    void loadWaves(const std::string& path);

    // Обновление системы волн
    void update(float deltaTime, std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<sf::Vector2i>& path);

    // Запуск волны
    void startWave();

    // Проверка завершения всех волн
    bool isFinished() const;

    // Получение текущего состояния
    WaveState getState() const;

    // Получение таймера паузы
    float getWaitTimer() const;

    // Получение индекса текущей волны
    int getCurrentWave() const;

    // Получение данных волны по индексу
    Wave getWave(int waveIndex) const;
};
