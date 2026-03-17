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
	int health;               // Количество жизней врага
	int speed;                // Скорость передвижения врага
	bool alive;               // Флаг жив/мертв врага
	bool reachedBase = false; // Флаг дошел до базы или нет
	int maxHealth;            // ХП врага

	sf::Vector2f pos;         // Текущая позиция врага
	sf::Vector2f offset;      // Смещение врага относительно центра пути
	int pathIndex = 0; 
	const std::vector<sf::Vector2i>* path;

public:
	Enemy(EnemyType type, int health, int speed, const std::vector<sf::Vector2i>& path);
	void update(float deltaTime);
	void render(sf::RenderWindow& window, sf::Vector2f mapOffset);

	sf::Vector2f getVelocity() const;

	bool isAlive() const;
	bool hasReachedBase() const;
	bool isKilled() const;  // умер от урона, не дошёл до базы

	sf::Vector2f getPos() const;
	void takeDamage(int damage);
	EnemyType getType() const;

	int getPathIndex() const;
};