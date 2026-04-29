#pragma once
#include "Enemy.hpp"
#include "GameData.hpp"
#include "UpgradeManager.hpp"
#include <Projectile.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <list>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС TOWER
//
///////////////////////////////////////////////////////////////////////////

class Tower {
public:
    static constexpr float IN_GAME_BONUS_STEP = 0.1f;

private:
    std::string typeSlug; // slug типа башни
    TowerStats stats; // статы башни
    sf::Vector2i gridPos; // позиция на сетке
    int enemyIndex = -1; // индекс выбранного врага
    float fireTimer = 0.f; // таймер стрельбы
    float currentAngle = 0.f; // текущий угол поворота турели
    float rotationSpeed = 180.f; // скорость поворота турели
    
    int inGameLevel = 1;      // Текущий уровень внутри боя
    int maxInGameLevel = 1;   // Максимально доступный уровень
    int totalInvested = 0;    // Суммарно потрачено монет на эту башню

    const sf::Texture* textureBase = nullptr; // ссылка на текстуру base
    const sf::Texture* textureTower = nullptr; // ссылка на текстуру tower

public:
    // Конструктор башни
    Tower(const std::string& slug, sf::Vector2i gridPos, UpgradeManager& upgradeManager);

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

    // Получение всей вложенной суммы
    int getTotalValue() const;

    // Прокачка внутри игры
    void upgradeInGame(int cost);
    int getInGameUpgradeCost() const;
    int getInGameLevel() const;
    int getMaxInGameLevel() const;
    bool canUpgradeInGame() const;
};
