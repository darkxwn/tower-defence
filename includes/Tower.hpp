#pragma once
#include <SFML/Graphics.hpp>
#include "Enemy.hpp"
#include "GameData.hpp"
#include <cmath>
#include <list>
#include <Projectile.hpp>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС TOWER
//
///////////////////////////////////////////////////////////////////////////

class Tower {
private:
    std::string typeSlug; // slug типа башни
    TowerStats stats; // статы башни
    sf::Vector2i gridPos; // позиция на сетке
    int enemyIndex = -1; // индекс выбранного врага
    float fireTimer = 0.f; // таймер стрельбы
    float currentAngle = 0.f; // текущий угол поворота турели
    float rotationSpeed = 180.f; // скорость поворота турели

    const sf::Texture* textureBase = nullptr; // ссылка на текстуру base
    const sf::Texture* textureTower = nullptr; // ссылка на текстуру tower

public:
    Tower(const std::string& slug, sf::Vector2i gridPos);

    // Обновление башни
    void update(float deltaTime, std::vector<std::unique_ptr<Enemy>>& enemies, std::vector<Projectile>& projectiles, sf::Vector2f mapOffset);

    // Отрисовка башни
    void render(sf::RenderWindow& window, sf::Vector2f mapOffset, bool showRadius = false);

    // Получение позиции на сетке
    sf::Vector2i getGridPos() const;

    // Получение типа башни
    std::string getTypeSlug() const;

    // Получение стоимости башни
    int getCost() const;
};
