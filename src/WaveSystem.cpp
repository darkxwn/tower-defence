#include "WaveSystem.hpp"
#include "GameData.hpp"
#include "utils/FileReader.hpp"
#include "Enemy.hpp"
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
    waves.clear();
    currentWave = 0;
    state = WaveState::Idle;

    auto content = readFile(path);

    std::istringstream file(content.value());
    std::string line;
    bool parsingWaves = false;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        // Ищем начало секции волн
        if (line.find("waves=") != std::string::npos) {
            parsingWaves = true;
            continue;
        }

        // Если нашли секцию — читаем данные волн
        if (parsingWaves) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string typeStr = line.substr(0, colon);
                int count = std::stoi(line.substr(colon + 1));

                EnemyType type = EnemyType::Basic;
                if (typeStr == "fast") type = EnemyType::Fast;
                else if (typeStr == "strong") type = EnemyType::Strong;

                waves.push_back({ type, count });
            }
        }
    }
}

void WaveSystem::update(float deltaTime, std::list<std::shared_ptr<Enemy>>& enemies, const std::vector<sf::Vector2i>& path) {
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

        auto stats = GameData::getEnemy(wave.type);
        enemies.push_back(std::make_shared<Enemy>(wave.type, stats.health, stats.speed, path));


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

Wave WaveSystem::getWave(int waveIndex) const { 
    if (waveIndex < 0 || waveIndex >= waves.size())
        throw std::runtime_error("[Ошибка]: Неверный индекс волны ");
    return waves[waveIndex];
}
