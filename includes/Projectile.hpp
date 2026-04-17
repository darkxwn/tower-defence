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
    sf::Vector2f pos; // текущая позиция
    Enemy* target = nullptr; // цель снаряда
    float speed; // скорость полёта
    int damage; // урон
    int splashRadius; // радиус урона по области
    bool alive = true; // состояние жизни
    std::string typeSlug; // slug типа башни

public:
    Projectile(sf::Vector2f startPos, Enemy* targetEnemy, int dmg, float spd, int splash, std::string slug);

    // Обновление снаряда
    void update(float deltaTime, std::vector<std::unique_ptr<Enemy>>& enemies);

    // Отрисовка снаряда
    void render(sf::RenderWindow& window, sf::Vector2f mapOffset);

    // Проверка состояния жизни
    bool isAlive() const;
};
