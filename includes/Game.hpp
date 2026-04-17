#pragma once
#include "Base.hpp"
#include "Enemy.hpp"
#include "HUD.hpp"
#include "Map.hpp"
#include "SettingsManager.hpp"
#include "Tower.hpp"
#include "WaveSystem.hpp"
#include "ui/Container.hpp"
#include "ui/Button.hpp"
#include "ui/Text.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС GAME
//
///////////////////////////////////////////////////////////////////////////

// Внутренние состояния игровой сессии
enum class GameState {
    Playing,
    Paused,
    GameOver,
    Victory
};

// Причина завершения сессии
enum class GameEndReason {
    None,
    ReturnToMenu,
    Restart,
    Win,
    Lose
};

class Game {
private:
    sf::RenderWindow& window;                // ссылка на окно отрисовки
    SettingsManager& settings;               // ссылка на настройки
    sf::Clock clock;                         // часы для расчета дельты времени

    sf::View worldView;                      // камера игрового мира
    sf::View uiView;                         // камера интерфейса
    sf::Vector2i lastInputPos;               // позиция ввода в прошлом кадре
    bool isPanning = false;                  // флаг перемещения камеры
    bool isPinching = false;                 // флаг зума пальцами
    float initialPinchDistance = 0.f;        // исходное расстояние при зуме
    float currentZoom = 1.0f;                // текущий масштаб мира
    float uiScale = 1.0f;                    // масштаб интерфейса
    bool hasMoved = false;                   // флаг того, что курсор двигался
    sf::Vector2i startTouchPos;              // начальная точка касания

    GameState state = GameState::Playing;    // текущее состояние игры
    GameEndReason endReason = GameEndReason::None; // причина выхода

    HUD hud;                                 // объект интерфейса
    Map map;                                 // объект карты
    int money = 0;                           // текущее количество денег
    Base base;                               // объект базы

    std::vector<std::unique_ptr<Enemy>> enemies; // список активных врагов
    std::vector<Tower> towers;                   // список построенных башен
    std::vector<Projectile> projectiles;         // список летящих снарядов

    WaveSystem waveSystem;                   // система управления волнами

    std::unique_ptr<UI::Container> pauseOverlay; // корневой контейнер паузы
    std::unique_ptr<UI::Container> endOverlay;   // корневой контейнер финала

    UI::Container* pauseModalPtr = nullptr;      // указатель на центральную плашку паузы
    UI::Container* endModalPtr = nullptr;        // указатель на центральную плашку финала

    UI::Text* endTitlePtr = nullptr;             // указатель на заголовок финала
    UI::Text* endSubTitlePtr = nullptr;          // указатель на подзаголовок финала

    // Основной цикл обновления логики
    void update(float deltaTime);

    // Основной цикл отрисовки
    void render();

    // Обработка всех событий сессии
    void handleEvents();

    // Настройка размеров камер
    void updateViewSizes(sf::Vector2u windowSize);

    // Логика взаимодействия с миром
    void processInput(sf::Vector2i pixelPos);

    // Инициализация интерфейса оверлеев
    void initOverlays();

public:
    // Конструктор загружает уровень и инициализирует игру
    Game(sf::RenderWindow& window, SettingsManager& settings, const std::string& levelPath);

    // Запускает выполнение игровой сессии
    void run();

    // Возвращает причину завершения игры
    GameEndReason getEndReason() const;

    // Ограничивает перемещение камеры границами карты
    void clampView();

    // Очистка ресурсов перед выходом
    void cleanup();
};
