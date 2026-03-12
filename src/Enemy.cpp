#include "Enemy.hpp"
#include "ResourceManager.hpp"
#include <cstdlib>
#include <iostream>

Enemy::Enemy(EnemyType type, int health, int speed, const std::vector<sf::Vector2i>& path)
    : type(type), health(health), speed(speed), alive(true), path(&path)
{
    pos = sf::Vector2f(path[0] * 64);
    offset = {
        (float)(rand() % 30) - 15.f,
        (float)(rand() % 30) - 15.f
    };
}

void Enemy::update(float deltaTime) {
    // враг дошел до базы
    if (pathIndex >= (int)path->size()) {
        reachedBase = true;
        alive = false;
        return;
    }

    sf::Vector2f target = sf::Vector2f((*path)[pathIndex] * 64);
    sf::Vector2f dir = target - pos;
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (length < 2.f) {
        // достигли точки — переходим к следующей
        pathIndex++;
    } else {
        dir /= length;
        pos += dir * (float)speed * deltaTime;
    }
}

void Enemy::render(sf::RenderWindow& window) {
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
    sf::Sprite sprite(ResourceManager::get(texName));
    sprite.setPosition(pos + offset + sf::Vector2f(16.f, 16.f));
    window.draw(sprite);
}

bool Enemy::isAlive() const {
    return alive;
}

bool Enemy::hasReachedBase() const {
    return reachedBase;
}