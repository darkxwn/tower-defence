#pragma once
#include <SFML/Graphics.hpp>

enum class EnemyType {
	Basic,
	Fast,
	Strong
};

class Enemy {
private:
	EnemyType type;           // Тип врага
	int health;               // Количество жизней
	int speed;                // Скоростьы
	bool alive;               // Флаг жив/мертв
	bool reachedBase = false; // Флаг дошел до базы или нет

	sf::Vector2f pos;         // Текущая позиция
	sf::Vector2f offset;      // Смещение относительно центра пути
	int pathIndex = 0; 
	const std::vector<sf::Vector2i>* path;

public:
	Enemy(EnemyType type, int health, int speed, const std::vector<sf::Vector2i>& path);
	void update(float deltaTime);
	void render(sf::RenderWindow& window);
	bool isAlive() const;
	bool hasReachedBase() const;
};