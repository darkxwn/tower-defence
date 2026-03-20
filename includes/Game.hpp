#pragma once
#include "Base.hpp"
#include "Enemy.hpp"
#include "HUD.hpp"
#include "Map.hpp"
#include "Tower.hpp"
#include "WaveSystem.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

// Перечисление игровых состояний
enum class GameState {
	MainMenu,
	LevelSelect,
	Settings,
	Upgrades,
	Playing,
	Paused,
	GameOver,
	Victory
};

// Класс игры
class Game {
private:
	sf::RenderWindow window;
	sf::Clock clock;
	sf::Vector2u windowSize = window.getSize();

	sf::RectangleShape pauseMenuBtn;
	sf::RectangleShape pauseRestartBtn;
	sf::RectangleShape pauseContinueBtn;

	GameState gameState = GameState::Playing;

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