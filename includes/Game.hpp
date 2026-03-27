#pragma once
#include "Base.hpp"
#include "Enemy.hpp"
#include "HUD.hpp"
#include "Map.hpp"
#include "Tower.hpp"
#include "WaveSystem.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// Игровые состояния (только внутриигровые; меню — отдельный класс Menu)
enum class GameState {
    Playing,
    Paused,
    GameOver,
    Victory
};

// Класс игровой сессии
// Принимает путь к .map файлу и запускает уровень
class Game {
private:
    sf::RenderWindow& window;
    sf::Clock clock;

    GameState gameState = GameState::Playing;

    HUD hud;
    Map map;
    int money = 0;
    Base base;
    std::vector<Enemy> enemies;
    std::vector<Tower> towers;
    WaveSystem waveSystem;

    // Кнопки оверлея паузы
    sf::RectangleShape pauseMenuBtn;
    sf::RectangleShape pauseRestartBtn;
    sf::RectangleShape pauseContinueBtn;

    void update(float deltaTime);
    void render();
    void handleEvents();

public:
    // Принимает ссылку на окно и путь к файлу уровня
    Game(sf::RenderWindow& window, const std::string& levelPath);

    // Запускает игровой цикл; возвращает управление когда игра завершена
    void run();

    // Результат сессии — нужно ли вернуться в меню
    bool shouldReturnToMenu() const;

private:
    bool returnToMenu = false;
};
