#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС ENEMY
// Объект врага, движущийся по заданному пути.
//
///////////////////////////////////////////////////////////////////////////

class Enemy {
private:
	std::string type; // идентификатор типа врага (строка)
	int health; // текущее здоровье
	int speed; // скорость движения
	bool alive; // флаг жизни
	bool reachedBase = false; // флаг достижения конца пути
	int maxHealth; // исходное здоровье для отрисовки HP-бара

	sf::Vector2f pos; // мировые координаты
	sf::Vector2f offset; // случайное смещение для имитации толпы
	int pathIndex = 0; // текущая цель на пути
	const std::vector<sf::Vector2i>* path; // ссылка на узлы пути

	const sf::Texture& texture; // ссылка на текстуру

public:
	// Конструктор инициализирует врага строковым типом и характеристиками
	Enemy(const std::string& type, int health, int speed, const std::vector<sf::Vector2i>& path);

	// Обновление логики движения
	void update(float deltaTime);

	// Отрисовка врага и его здоровья
	void render(sf::RenderWindow& window, sf::Vector2f mapOffset);

	// Получение вектора скорости
	sf::Vector2f getVelocity() const;

	// Статус жизни
	bool isAlive() const;

	// Статус проникновения на базу
	bool hasReachedBase() const;

	// Статус гибели от башни
	bool isKilled() const;

	// Позиция в мире
	sf::Vector2f getPos() const;

	// Нанесение урона врагу
	void takeDamage(int damage);

	// Получение идентификатора типа
	std::string getType() const;

	// Индекс текущей точки пути
	int getPathIndex() const;
};
