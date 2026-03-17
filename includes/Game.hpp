#pragma once
#include "Base.hpp"
#include "Enemy.hpp"
#include "Map.hpp"
#include "WaveSystem.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include "HUD.hpp"
#include "Tower.hpp"

// Класс игры
class Game {
private:
	sf::RenderWindow window;
	sf::Clock clock;
	sf::Vector2u windowSize = window.getSize();

	HUD hud;
	Map map;
	int money = 0;
	Base base;
	std::vector<Enemy> enemies;
	std::vector<Tower> towers;
	WaveSystem waveSystem;
	
	void update(float deltaTime);
	void render();
	void handleEvents();

public: 
	Game();
	void run();
	
};