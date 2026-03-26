#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>

// Состояния меню
enum class MenuState {
    Main,        // Главное меню
    LevelSelect, // Выбор уровня
    Upgrades,    // Улучшения (заглушка)
    Settings     // Настройки (заглушка)
};

// Информация об уровне, считанная из .map файла
struct LevelInfo {
    std::string filePath;  // Путь к .map файлу
    std::string name;      // Название уровня из файла
    int index;             // Порядковый номер уровня
};

// Вспомогательная структура кнопки меню
struct MenuButton {
    sf::RectangleShape shape;
    std::string label;
    std::function<void()> onClick;
};

// Класс главного меню игры
// Управляет переходами между подменю и запуском уровней
class Menu {
private:
    sf::RenderWindow& window;
    MenuState state = MenuState::Main;

    std::vector<LevelInfo> levels; // Загруженные уровни
    std::string selectedLevel;     // Путь к выбранному уровню (пустой = не выбрано)
    bool levelChosen = false;      // Флаг: игрок нажал "Играть"

    // --- Отрисовка подменю ---
    void renderMain();
    void renderLevelSelect();
    void renderStub(const std::string& title); // Заглушка для Settings/Upgrades

    // --- Обработка кликов ---
    void handleMainClick(sf::Vector2f pos);
    void handleLevelSelectClick(sf::Vector2f pos);

    // --- Вспомогательные методы ---
    void scanLevels();                               // Сканирование папки data/levels/
    std::string readLevelName(const std::string& path) const; // Читает поле name= из .map

    // Рисует кнопку меню; enabled=false делает её визуально неактивной
    sf::FloatRect drawButton(const std::string& label, sf::Vector2f pos, sf::Vector2f size,
                             bool hovered = false, bool enabled = true);

public:
    explicit Menu(sf::RenderWindow& window);

    void handleEvents();
    void render();

    // Вернёт true, если игрок выбрал уровень и нажал "Играть"
    bool isLevelChosen() const;

    // Путь к выбранному .map файлу
    std::string getChosenLevel() const;

    // Сбросить выбор (вызывается после запуска игры)
    void resetChoice();
};
