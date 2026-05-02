#pragma once
#include "Base.hpp"
#include "Enemy.hpp"
#include "HUD.hpp"
#include "Map.hpp"
#include "SettingsManager.hpp"
#include "SaveManager.hpp"
#include "UpgradeManager.hpp"
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
    // Интерфейсные оверлеи (в начале для защиты от порчи памяти и корректного порядка удаления)
    std::unique_ptr<UI::Container> pauseOverlay;
    std::unique_ptr<UI::Container> endOverlay;

    sf::RenderWindow& window;                // ссылка на окно отрисовки
    SettingsManager& settings;               // ссылка на настройки
    sf::Clock clock;                         // часы для расчета дельты времени

    sf::View worldView;                      // камера игрового мира
    sf::View uiView;                         // камера интерфейса
    sf::Vector2i lastInputPos;               // позиция ввода в прошлом кадре
    sf::Vector2i startTouchPos;              // начальная точка касания

    // Простые параметры
    float currentZoom = 1.0f;                // текущий масштаб мира
    float minZoom = 0.5f;                    // лимит приближения (вычисляется динамически)
    float maxZoom = 1.5f;                    // лимит отдаления (вычисляется динамически)
    float uiScale = 1.0f;                    // масштаб интерфейса
    float initialPinchDistance = 0.f;        // дистанция зума
    bool isPanning = false;
    bool isPinching = false;
    bool hasMoved = false;
    int money = 0;
    int currentScore = 0;
    int accumulatedGlobalMoney = 0; // накопленные глобальные деньги за уровень
    std::string levelId; // идентификатор текущего уровня

    GameState state = GameState::Playing;
    GameEndReason endReason = GameEndReason::None;

    // Объекты систем
    HUD hud;
    Map map;
    Base base;
    WaveSystem waveSystem;
    UpgradeManager& upgradeManager;
    SaveManager& saveManager;

    // Коллекции объектов
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Tower> towers;
    std::vector<Projectile> projectiles;

    // Указатели на элементы оверлеев
    UI::Container* pauseModalPtr = nullptr;
    UI::Container* endModalPtr = nullptr;
    UI::Text* endTitlePtr = nullptr;
    UI::Text* endSubTitlePtr = nullptr;

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
    Game(sf::RenderWindow& window, SettingsManager& settings, SaveManager& saveManager, 
        UpgradeManager& upgradeManager, const std::string& levelPath);
    
    // Запускает выполнение игровой сессии
    void run();

    // Возвращает причину завершения игры
    GameEndReason getEndReason() const;

    // Ограничивает перемещение камеры границами карты
    void clampView();

    // Очистка ресурсов перед выходом
    void cleanup();
};
