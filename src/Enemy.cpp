#include "Enemy.hpp"
#include "ResourceManager.hpp"
#include <cstdlib>
#include <iostream>
#include "Colors.hpp"

Enemy::Enemy(EnemyType type, int health, int speed, const std::vector<sf::Vector2i>& path)
    : type(type), health(health), maxHealth(health), speed(speed), alive(true), path(&path)
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

void Enemy::render(sf::RenderWindow& window, sf::Vector2f mapOffset) {
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
    // позиция полоски — над спрайтом врага
    sf::Vector2f barPos = mapOffset + pos + offset + sf::Vector2f(16.0f, 8.f);
    float barWidth = 32.f;
    float barHeight = 4.f;
    // фон полоски
    sf::RectangleShape bgBar({ barWidth, barHeight });
    bgBar.setFillColor(Colors::hpBarBg);
    bgBar.setPosition(barPos);
    window.draw(bgBar);
    // заполненная часть — пропорционально текущему HP
    float ratio = (float)health / (float)maxHealth;
    sf::RectangleShape hpBar({ barWidth * ratio, barHeight });
    hpBar.setFillColor(Colors::hpBarFill);
    hpBar.setPosition(barPos);
    window.draw(hpBar);

    // спрайт врага
    sf::Sprite sprite(ResourceManager::get(texName));
    sprite.setScale({ 0.125f, 0.125f });
    sprite.setPosition(mapOffset + pos + offset + sf::Vector2f(16.f, 16.f));
    window.draw(sprite);
}

void Enemy::takeDamage(int damage) {
    health -= damage;
    if (health <= 0) {
        alive = false;
    }
}

// Считаем направление движения
sf::Vector2f Enemy::getVelocity() const {
    if (pathIndex >= (int)path->size()) return { 0.f, 0.f };
    sf::Vector2f target = sf::Vector2f((*path)[pathIndex] * 64);
    sf::Vector2f dir = target - pos;
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length < 0.1f) return { 0.f, 0.f };
    return (dir / length) * (float)speed;
}

bool Enemy::isAlive() const { return alive; }

bool Enemy::hasReachedBase() const { return reachedBase; }

EnemyType Enemy::getType() const { return type; }

sf::Vector2f Enemy::getPos() const { return pos; }

bool Enemy::isKilled() const { return !alive && !reachedBase; }

int Enemy::getPathIndex() const { return pathIndex; }