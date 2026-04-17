#pragma once
#include "SettingsManager.hpp"
#include "ui/Container.hpp"
#include "ui/Text.hpp"
#include "ui/Button.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС MENU
//
///////////////////////////////////////////////////////////////////////////

// Состояния экранов меню
enum class MenuState {
    Main,
    LevelSelect,
    Upgrades,
    Settings
};

// Результат завершённой игровой сессии
enum class SessionResult {
    None,
    Win,
    Lose
};

// Информация об уровне из файла карты
struct LevelInfo {
    std::string filePath; // путь к файлу .map
    std::string name; // название уровня
    int index; // порядковый номер
};

class Menu {
private:
    sf::RenderWindow& window; // ссылка на окно отрисовки
    SettingsManager& settings; // ссылка на менеджер настроек
    MenuState state = MenuState::Main; // текущее состояние меню

    std::vector<LevelInfo> levels; // список доступных уровней
    std::string selectedLevel; // путь к выбранной карте
    bool levelChosen = false; // уровень выбран

    sf::View worldView; // камера заднего плана
    sf::View uiView; // камера интерфейса
    float uiScale = 1.0f; // масштаб интерфейса
    bool windowRecreationRequired = false; // запрос пересоздания окна

    std::unique_ptr<UI::Container> mainContainer; // контейнер главного меню
    std::unique_ptr<UI::Container> levelContainer; // контейнер выбора уровня
    std::unique_ptr<UI::Container> settingsContainer; // контейнер настроек
    std::unique_ptr<UI::Container> upgradesContainer; // контейнер улучшений
    std::unique_ptr<UI::Container> resultOverlay; // оверлей результата игры

    // Указатели для управления динамическим ресайзом
    UI::Container* cardsArea = nullptr; // область карточек уровней
    UI::Button* playBtnPtr = nullptr; // кнопка старта игры
    UI::Container* headerContPtr = nullptr; // блок заголовка главного меню
    UI::Container* btnsContPtr = nullptr; // блок кнопок главного меню
    UI::Text* titleTextPtr = nullptr; // текст названия игры

    SessionResult lastResult = SessionResult::None; // итог последней игры
    std::string lastLevelPath; // путь к последней сыгранной карте

    // Инициализация всех контейнеров и их содержимого
    void initUI();

    // Создание типового подменю с заголовком и навигацией
    std::unique_ptr<UI::Container> createSubMenu(const std::string& title, UI::Container** outContent, UI::Container** outNav = nullptr);

    // Обновление внешнего вида карточек при изменении выбора
    void updateCardsSelection();

    // Сканирование папки с файлами уровней
    void scanLevels();

    // Чтение названия уровня из метаданных файла
    std::string readLevelName(const std::string& path) const;

public:
    // Конструктор инициализирует ресурсы и интерфейс
    explicit Menu(sf::RenderWindow& window, SettingsManager& settings);

    // Обработка системных событий меню и кликов
    void handleEvents();

    // Главный метод отрисовки текущего состояния
    void render();

    // Получение факта выбора уровня
    bool isLevelChosen() const;

    // Получение пути к выбранному файлу карты
    std::string getChosenLevel() const;

    // Сброс состояния выбора уровня
    void resetChoice();

    // Сброс результата последней игры
    void resetLastResult();

    // Проверка необходимости пересоздания окна
    bool consumesWindowRecreationRequest();

    // Обновление параметров камер и размеров контейнеров
    void updateViewSizes(sf::Vector2u windowSize);

    // Принятие результата игры для отображения баннера
    void notifyResult(SessionResult result, const std::string& levelPath);

    // Очистка всех ресурсов перед закрытием окна
    void cleanup();
};
