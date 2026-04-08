#pragma once
#include <SFML/Graphics.hpp>
#include "Enemy.hpp"
#include "GameData.hpp"
#include <cmath>
#include <Projectile.hpp>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС TOWER
//
///////////////////////////////////////////////////////////////////////////

class Tower {
private:
    std::string typeSlug;      
    TowerStats stats;
    sf::Vector2i gridPos;
    int enemyIndex = -1;
    float fireTimer = 0.f;
    float currentAngle = 0.f;
    float rotationSpeed = 180.f;

public:
    // Конструктор теперь принимает строку имени типа
    Tower(const std::string& slug, sf::Vector2i gridPos);

    void update(float deltaTime, std::list<std::shared_ptr<Enemy>>& enemies, std::vector<Projectile>& projectiles, sf::Vector2f mapOffset);
    void render(sf::RenderWindow& window, sf::Vector2f mapOffset, bool showRadius = false);

    sf::Vector2i getGridPos() const;
};