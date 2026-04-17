#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС ENEMY
//
///////////////////////////////////////////////////////////////////////////

// Типы врагов
enum class EnemyType {
	Basic,
	Fast,
	Strong
};

class Enemy {
private:
	EnemyType type; // тип врага
	int health; // количество жизней
	int speed; // скорость передвижения
	bool alive; // состояние жизни
	bool reachedBase = false; // враг достиг базы
	int maxHealth; // максимальное количество жизней

	sf::Vector2f pos; // текущая позиция
	sf::Vector2f offset; // смещение относительно центра пути
	int pathIndex = 0; // индекс текущего узла пути
	const std::vector<sf::Vector2i>* path; // указатель на массив точек пути

public:
	Enemy(EnemyType type, int health, int speed, const std::vector<sf::Vector2i>& path);

	// Обновление состояния врага
	void update(float deltaTime);

	// Отрисовка врага
	void render(sf::RenderWindow& window, sf::Vector2f mapOffset);

	// Получение вектора текущего направления движения
	sf::Vector2f getVelocity() const;

	// Проверка жизни
	bool isAlive() const;

	// Проверка достижения базы
	bool hasReachedBase() const;

	// Проверка гибели от урона
	bool isKilled() const;

	// Получение текущей позиции
	sf::Vector2f getPos() const;

	// Получение урона
	void takeDamage(int damage);

	// Получение типа врага
	EnemyType getType() const;

	// Получение индекса текущего узла пути
	int getPathIndex() const;
};
