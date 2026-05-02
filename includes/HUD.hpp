#pragma once 
#include <SFML/Graphics.hpp>
#include "WaveSystem.hpp"
#include "ui/Button.hpp" 
#include "ui/Text.hpp"

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС HUD
//
///////////////////////////////////////////////////////////////////////////

class HUD {
private:
    float uiScale = 1.5f; // коэффициент увеличения интерфейса
    std::vector<UI::Button> towerSlots; // кнопки выбора башен
    UI::Button pauseBtn; // кнопка вызова паузы
    UI::Button skipBtn; // кнопка старта волны
    UI::Button speedBtn; // кнопка переключения скорости

    // Управление выбранной башней
    UI::Button upgradeBtn; // кнопка улучшения
    UI::Button sellBtn; // кнопка продажи
    bool showTowerMenu = false; // флаг отображения меню управления башней
    bool sellRequested = false; // флаг запроса продажи
    bool upgradeRequested = false; // флаг запроса улучшения

    int currentSellPrice = 0;
    int currentUpgradePrice = 0;
    bool canUpgrade = false;

    // Логика подтверждения (даблклик)
    sf::Clock doubleClickClock;
    int lastClickedBtnId = 0; // 0 - ни одна, 1 - улучшение, 2 - продажа
    const float doubleClickTime = 0.4f; // время на второй клик (сек)

    int speedMode = 0; // режим ускорения
    bool pauseRequested = false; // запрос паузы
    bool skipRequested = false; // запрос старта волны
    int selectedTowerSlot = -1; // индекс выбранной башни

    UI::Text scoreText; // отображение очков

    const sf::Texture* coinTex = nullptr; // иконка монет
    const sf::Texture* heartTex = nullptr; // иконка жизней
    const sf::Font* mainFont = nullptr; // основной шрифт
    const sf::Texture* speedTexs[3]; // массив текстур для режимов скорости

public:
    // Конструктор инициализирует элементы интерфейса
    HUD();

    // Отрисовывает все элементы интерфейса
    void render(sf::RenderWindow& window, int money, int lives, int wave, WaveState state, int currentScore);

    // Обновляет позиции элементов при изменении размеров экрана
    void updateLayout(sf::Vector2f viewSize, float uiScale = 1.0f);

    // Установить масштаб интерфейса
    void setUiScale(float scale);

    // Обрабатывает события нажатий и наведения для кнопок. 
    // Возвращает true, если событие было поглощено интерфейсом.
    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView);

    // Сбрасывает выделенный слот в магазине
    void resetSelectedSlot();

    // Показывает меню управления башней в указанных экранных координатах
    void showTowerControls(sf::Vector2f screenPos, int sellPrice, int upgradePrice, bool canUpgrade, float worldZoom = 1.0f);

    // Скрывает меню управления башней
    void hideTowerControls();

    // Проверка запроса на продажу
    bool isSellRequested() const;

    // Проверка запроса на улучшение
    bool isUpgradeRequested() const;

    // Сброс флагов запросов
    void resetRequests();

    // Получение множителя скорости игры
    float getGameSpeed() const;

    // Проверка запроса на паузу
    bool isPauseRequested() const;

    // Проверка запроса на старт волны
    bool isSkipRequested() const;

    // Получение индекса выбранного слота
    int getSelectedSlot() const;
};
