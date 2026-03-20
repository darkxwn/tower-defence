#pragma once
#include <SFML/Graphics.hpp>
#include "Enemy.hpp"
#include "GameData.hpp"
#include <cmath>

enum class TowerType {
	Basic,
	Cannon,
	Double,
	Sniper,
};

class Tower {
private:
	TowerType type;            // Тип башни
	TowerStats stats;          // Статы башни
	sf::Vector2i gridPos;      // Позиция в тайлах
	int enemyIndex = -1;             // Указатель на врага

	float fireTimer = 0.f;     // Таймер стрельбы

	sf::Angle angle;           // Угол поворота
	float currentAngle = 0.f;    // текущий визуальный угол в градусах
	float rotationSpeed = 360.f; // скорость поворота — градусов в секунду
public:
	Tower(TowerType type, sf::Vector2i gridPos);

	void update(float deltaTime, std::vector<Enemy>& enemies, sf::Vector2f mapOffset);
	void render(sf::RenderWindow& window, sf::Vector2f mapOffset, bool showRadius = false);

	sf::Vector2i getGridPos() const;

	
};

