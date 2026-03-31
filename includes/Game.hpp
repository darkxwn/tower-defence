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

// Внутренние состояния игровой сессии
enum class GameState {
    Playing,
    Paused,
    GameOver,  // поражение — база уничтожена
    Victory    // победа — все волны пройдены
};

// Причина завершения сессии (возвращается в main)
enum class GameEndReason {
    None,          // сессия ещё идёт
    ReturnToMenu,  // игрок нажал "Главное меню"
    Restart,       // игрок нажал "Заново"
    Win,           // победа
    Lose           // поражение
};

// Игровая сессия одного уровня
class Game {
private:
    sf::RenderWindow& window;
    sf::Clock clock;

    GameState state = GameState::Playing;
    GameEndReason endReason = GameEndReason::None;

    HUD hud;
    Map map;
    int money = 0;
    Base base;
    std::vector<Enemy> enemies;
    std::vector<Tower> towers;
    WaveSystem waveSystem;

    // Кнопки оверлея паузы (позиции вычисляются в render)
    sf::FloatRect pauseMenuRect;
    sf::FloatRect pauseRestartRect;
    sf::FloatRect pauseContinueRect;

    // Кнопки экранов победы/поражения
    sf::FloatRect endMenuRect;
    sf::FloatRect endRestartRect;

    void update(float deltaTime);
    void render();
    void handleEvents();

    void renderPauseOverlay();
    void renderEndScreen();   // общий экран для Victory и GameOver

    // Вычисляет позиции трёх кнопок паузы по центру экрана
    void computePauseBtnLayout();

public:
    Game(sf::RenderWindow& window, const std::string& levelPath);

    // Запускает цикл уровня; возвращает управление когда сессия завершена
    void run();

    GameEndReason getEndReason() const;
};
