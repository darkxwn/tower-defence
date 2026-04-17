#include "WaveSystem.hpp"
#include "GameData.hpp"
#include "utils/FileReader.hpp"
#include "Enemy.hpp"
#include <fstream>
#include <sstream>

// Интервал спавна по типу врага
float WaveSystem::getSpawnInterval(EnemyType type) const {
    switch (type) {
        case EnemyType::Fast:   return 0.7f;
        case EnemyType::Strong: return 1.0f;
        default:                return 0.5f;
    }
}

// Загрузка волн из файла
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

        if (line.find("waves=") != std::string::npos) {
            parsingWaves = true;
            continue;
        }

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

// Обновление системы волн
void WaveSystem::update(float deltaTime, std::vector<std::unique_ptr<Enemy>>& enemies, const std::vector<sf::Vector2i>& path) {
    if (currentWave >= (int)waves.size()) return;

    if (state == WaveState::Idle) return;

    // пауза между волнами
    if (state == WaveState::Waiting) {
        waitTimer -= deltaTime;
        if (waitTimer <= 0.f)
            startWave();
        return;
    }

    // ожидание завершения текущей волны
    if (state == WaveState::Fighting) {
        if (enemies.empty()) {
            currentWave++;
            if (currentWave >= (int)waves.size()) return;
            state = WaveState::Waiting;
            waitTimer = waitInterval;
        }
        return;
    }

    // спавн врагов текущей волны
    if (state == WaveState::Spawning) {
        Wave& wave = waves[currentWave];

        spawnTimer += deltaTime;
        if (spawnTimer < getSpawnInterval(wave.type)) return;
        spawnTimer = 0.f;

        auto stats = GameData::getEnemy(wave.type);
        enemies.push_back(std::make_unique<Enemy>(wave.type, stats.health, stats.speed, path));

        spawned++;

        // переход в ожидание завершения при полном спавне
        if (spawned >= wave.count) {
            spawned = 0;
            state = WaveState::Fighting;
        }
    }
}

// Запуск волны
void WaveSystem::startWave() {
    if (state != WaveState::Idle && state != WaveState::Waiting) return;
    state = WaveState::Spawning;
    spawnTimer = 0.f;
}

// Проверка завершения всех волн
bool WaveSystem::isFinished() const {
    return currentWave >= (int)waves.size();
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

// Получение данных волны по индексу
Wave WaveSystem::getWave(int waveIndex) const {
    if (waveIndex < 0 || waveIndex >= (int)waves.size())
        throw std::runtime_error("[Ошибка]: Неверный индекс волны ");
    return waves[waveIndex];
}
