#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

enum class MenuState { Main, LevelSelect, Upgrades, Settings };

// Результат завершённой игровой сессии
enum class SessionResult { None, Win, Lose };

// Информация об уровне из .map файла
struct LevelInfo {
    std::string filePath;
    std::string name;
    int index; // 0-based порядковый номер
};

// Класс главного меню.
// Ключевой принцип надёжности кликов: вся геометрия кнопок/карточек
// вычисляется один раз через compute*Layout() и передаётся ОДНОВРЕМЕННО
// в render и handleClick — позиции всегда совпадают.
class Menu {
public:
    struct MainLayout {
        static constexpr int BTN_COUNT = 4;
        sf::FloatRect btns[BTN_COUNT]; // 0=Играть 1=Улучшения 2=Настройки 3=Выход
    };

    struct CardLayout {
        sf::FloatRect bounds;
        int levelIndex; // индекс в levels[]
    };

    struct LevelSelectLayout {
        std::vector<CardLayout> cards;
        sf::FloatRect playBtn;
        sf::FloatRect backBtn;
        int rowSize;     // карточек в строке
        int rowCount;    // строк
    };

private:
    sf::RenderWindow& window;
    MenuState state = MenuState::Main;

    std::vector<LevelInfo> levels;
    std::string selectedLevel; // путь к выбранному .map, пустой = нет выбора
    bool levelChosen = false;

    sf::View worldView; // Камера для игровых объектов
    sf::View uiView;    // Камера для кнопок и текста
    float currentZoom = 1.0f;
    float uiScale = 1.0f;

    // Метод для синхронизации размеров при старте и ресайзе

    SessionResult lastResult = SessionResult::None;
    std::string   lastLevelPath; // путь к последнему сыгранному уровню

    // Геометрия — вычисляется один раз за кадр
    MainLayout        computeMainLayout()        const;
    LevelSelectLayout computeLevelSelectLayout() const;

    // Отрисовка
    void renderMain(const MainLayout& L);
    void renderLevelSelect(const LevelSelectLayout& L);
    void renderResultOverlay(); // баннер победы/поражения поверх LevelSelect
    void renderStub(const std::string& title);
    void updateViewSizes(sf::Vector2u windowSize);

    // Обработка кликов
    void handleMainClick(sf::Vector2f pos, const MainLayout& L);
    void handleLevelSelectClick(sf::Vector2f pos, const LevelSelectLayout& L);

    // Утилиты
    void        scanLevels();
    std::string readLevelName(const std::string& path) const;
    void        drawBtn(const std::string& label, sf::FloatRect r,
        bool hovered, bool enabled = true,
        sf::Color customFill = sf::Color::Transparent) const;

public:
    explicit Menu(sf::RenderWindow& window);

    void handleEvents();
    void render();

    bool        isLevelChosen()  const;
    std::string getChosenLevel() const;
    void        resetChoice();   // вызывать ПОСЛЕ getChosenLevel()

    // Вызывается из main после завершения игровой сессии
    void notifyResult(SessionResult result, const std::string& levelPath);
};