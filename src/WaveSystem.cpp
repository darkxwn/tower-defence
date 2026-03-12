// WaveSystem.cpp
#include "WaveSystem.hpp"
#include <fstream>
#include <sstream>

float WaveSystem::getSpawnInterval(EnemyType type) const {
    switch (type) {
        case EnemyType::Fast:   return 0.7f;
        case EnemyType::Strong: return 1.0f;
        default:                return 0.5f;
    }
}

void WaveSystem::loadWaves(const std::string& path) {
    std::ifstream file(path);
    std::string line;

    // пропускаем до секции waves=
    while (std::getline(file, line))
        if (line == "waves=") break;

    // каждая строка — одна волна, формат: basic:10
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;

        std::string typeName = line.substr(0, colon);
        int count = std::stoi(line.substr(colon + 1));

        EnemyType type;
        if (typeName == "basic")       type = EnemyType::Basic;
        else if (typeName == "fast")   type = EnemyType::Fast;
        else if (typeName == "strong") type = EnemyType::Strong;
        else continue;

        waves.push_back({ type, count });
    }
}

void WaveSystem::update(float deltaTime, std::vector<Enemy>& enemies, const std::vector<sf::Vector2i>& path) {
    if (currentWave >= (int)waves.size()) return;

    if (state == WaveState::Idle) return;

    // таймер между волнами
    if (state == WaveState::Waiting) {
        waitTimer -= deltaTime;
        if (waitTimer <= 0.f)
            startWave();
        return;
    }

    // ждём пока все враги убиты или дошли до базы
    if (state == WaveState::Fighting) {
        if (enemies.empty()) {
            currentWave++;
            if (currentWave >= (int)waves.size()) return;
            state = WaveState::Waiting;
            waitTimer = waitInterval;
        }
        return;
    }

    // спавним врагов текущей волны по одному
    if (state == WaveState::Spawning) {
        Wave& wave = waves[currentWave];

        spawnTimer += deltaTime;
        if (spawnTimer < getSpawnInterval(wave.type)) return;
        spawnTimer = 0.f;

        int speed = 64;
        if (wave.type == EnemyType::Fast)   speed = 128;
        if (wave.type == EnemyType::Strong) speed = 32;

        enemies.emplace_back(wave.type, 100, speed, path);
        spawned++;

        // все заспавнены — переходим в Fighting
        if (spawned >= wave.count) {
            spawned = 0;
            state = WaveState::Fighting;
        }
    }
}

void WaveSystem::startWave() {
    if (state != WaveState::Idle && state != WaveState::Waiting) return;
    state = WaveState::Spawning;
    spawnTimer = 0.f;
}

bool WaveSystem::isFinished() const {
    return currentWave >= (int)waves.size();
}

WaveState WaveSystem::getState() const { return state; }
float WaveSystem::getWaitTimer() const { return waitTimer; }
int WaveSystem::getCurrentWave() const { return currentWave; }
