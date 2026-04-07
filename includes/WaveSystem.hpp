#pragma once
#include "Enemy.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС WAVESYSTEM
//
///////////////////////////////////////////////////////////////////////////

struct Wave {
    EnemyType type;   // Тип врага
    int count;        // Количество врагов в группе
};

enum class WaveState {
    Idle,      // первая волна — ждём кнопку
    Waiting,   // между волнами — идёт таймер
    Spawning,  // спавним врагов
    Fighting   // все заспавнены — ждём пока убьют
};

class WaveSystem {
private:
    std::vector<Wave> waves;

    int currentWave = 0;  // индекс текущей волны
    int spawned = 0;      // сколько врагов уже заспавнено

    float spawnTimer = 0.f;
    float waitTimer = 0.f;
    float waitInterval = 8.f;  // пауза между волнами в секундах

    WaveState state = WaveState::Idle;

    float getSpawnInterval(EnemyType type) const;

public:
    void loadWaves(const std::string& path);
    void update(float deltaTime, std::vector<Enemy>& enemies, const std::vector<sf::Vector2i>& path);

    void startWave();
    bool isFinished() const;
    WaveState getState() const;
    float getWaitTimer() const;
    int getCurrentWave() const;
    Wave getWave(int waveIndex) const;
};