#pragma once
#include <SFML/Graphics.hpp>
#include <list>
#include "Enemy.hpp"

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС PROJECTILE
//
///////////////////////////////////////////////////////////////////////////

class Projectile {
private:
    sf::Vector2f pos;
    std::shared_ptr<Enemy> target; // цель снаряда
    float speed;
    int damage;
    int splashRadius;
    bool alive = true;
    std::string typeSlug; // идентификатор типа (basic, cannon...)

public:
    Projectile(sf::Vector2f startPos, std::shared_ptr<Enemy> targetEnemy, int dmg, float spd, int splash, std::string slug);

    void update(float deltaTime, std::list<std::shared_ptr<Enemy>>& enemies);
    void render(sf::RenderWindow& window, sf::Vector2f mapOffset);

    bool isAlive() const { return alive; }
};