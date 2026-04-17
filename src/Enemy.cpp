#include "Enemy.hpp"
#include "ResourceManager.hpp"
#include "utils/Logger.hpp"
#include "utils/Math.hpp"
#include "Colors.hpp"

// Конструктор врага
Enemy::Enemy(EnemyType type, int health, int speed, const std::vector<sf::Vector2i>& path)
    : type(type), health(health), maxHealth(health), speed(speed), alive(true), path(&path)
{
    pos = sf::Vector2f(path[0] * 64);
    offset = {
        Math::Random::getFloat(-15.f, 15.f),
        Math::Random::getFloat(-15.f, 15.f)
    };

    if (path.empty()) {
        LOGI("[ERROR]: путь для врагов не найден");
    }
}

// Обновление состояния врага
void Enemy::update(float deltaTime) {
    // проверка достижения конца пути
    if (pathIndex >= (int)path->size()) {
        reachedBase = true;
        alive = false;
        return;
    }

    // движение к следующей точке
    sf::Vector2f target = sf::Vector2f((*path)[pathIndex] * 64);
    sf::Vector2f dir = target - pos;
    float distSq = Math::getDistSq(pos, target);

    if (distSq < 4.f) {
        pathIndex++;
    } else {
        sf::Vector2f dir = Math::normalize(target - pos);
        pos += dir * (float)speed * deltaTime;
    }
}

// Отрисовка врага
void Enemy::render(sf::RenderWindow& window, sf::Vector2f mapOffset) {
    // выбор текстуры по типу
    std::string texName;
    switch (type) {
    case EnemyType::Basic:
        texName = "enemy-basic";
        break;
    case EnemyType::Fast:
        texName = "enemy-fast";
        break;
    case EnemyType::Strong:
        texName = "enemy-strong";
        break;
    }

    // позиция полоски жизней
    sf::Vector2f barPos = mapOffset + pos + offset + sf::Vector2f(16.0f, 8.f);
    float barWidth = 32.f;
    float barHeight = 4.f;

    // фон полоски
    sf::RectangleShape bgBar({ barWidth, barHeight });
    bgBar.setFillColor(Colors::Theme::HpBarBg);
    bgBar.setPosition(barPos);
    window.draw(bgBar);

    // заполнение полоски пропорционально hp
    float ratio = (float)health / (float)maxHealth;
    sf::RectangleShape hpBar({ barWidth * ratio, barHeight });
    hpBar.setFillColor(Colors::Theme::HpBarFill);
    hpBar.setPosition(barPos);
    window.draw(hpBar);

    // спрайт врага
    sf::Sprite sprite(ResourceManager::get(texName));
    sprite.setScale({ 0.11f, 0.11f });
    sprite.setPosition(mapOffset + pos + offset + sf::Vector2f(16.f, 16.f));
    window.draw(sprite);
}

// Получение урона
void Enemy::takeDamage(int damage) {
    health -= damage;
    if (health <= 0) {
        alive = false;
    }
}

// Получение вектора текущего направления движения
sf::Vector2f Enemy::getVelocity() const {
    if (path->empty() || pathIndex >= (int)path->size()) return { 0.f, 0.f };

    // расчёт направления к следующей точке
    int nextIdx = pathIndex;
    sf::Vector2f targetPos = sf::Vector2f((*path)[nextIdx] * 64);

    sf::Vector2f dir = targetPos - pos;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    // переход к следующей точке при близком расстоянии
    if (len < 1.0f && nextIdx + 1 < (int)path->size()) {
        nextIdx++;
        targetPos = sf::Vector2f((*path)[nextIdx] * 64);
        dir = targetPos - pos;
        len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    }

    if (len < 0.1f) return { 0.f, 0.f };
    return (dir / len) * (float)speed;
}

// Проверка жизни
bool Enemy::isAlive() const {
    return alive;
}

// Проверка достижения базы
bool Enemy::hasReachedBase() const {
    return reachedBase;
}

// Получение типа врага
EnemyType Enemy::getType() const {
    return type;
}

// Получение текущей позиции
sf::Vector2f Enemy::getPos() const {
    return pos;
}

// Проверка гибели от урона
bool Enemy::isKilled() const {
    return !alive && !reachedBase;
}

// Получение индекса текущего узла пути
int Enemy::getPathIndex() const {
    return pathIndex;
}

