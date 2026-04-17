#pragma once
#include <SFML/Graphics.hpp>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС BASE
//
///////////////////////////////////////////////////////////////////////////

class Base {
private:
	int baseLives = 20; // количество жизней базы
	sf::Vector2f basePos; // позиция базы
	bool destroyed = false; // состояние разрушения

public:
	Base(sf::Vector2i pos);

	// Получение урона
	void takeDamage(int damage);

	// Получение количества жизней
	int getLives() const;

	// Проверка состояния разрушения
	bool isDestroyed() const;
};