#pragma once
#include <SFML/Graphics.hpp>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС BASE
//
///////////////////////////////////////////////////////////////////////////

class Base {
private:
	int baseLives = 20;    // Количество жизней базы 
	sf::Vector2f basePos;
	bool destroyed = false;

public:
	Base(sf::Vector2i pos);
	void takeDamage(int damage);
	int getLives() const;
	bool isDestroyed() const;
};