#pragma once
#include "Enemy.hpp"
#include "Map.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include "Base.hpp"
#include "WaveSystem.hpp"

// Класс игры
class Game {
private:
	sf::RenderWindow window;
	sf::Clock clock;

	Map map;
	Base base;
	std::vector<Enemy> enemies;
	WaveSystem waveSystem;

	void update(float deltaTime);
	void render();
	void handleEvents();

public: 
	Game();
	void run();

};