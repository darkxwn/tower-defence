#pragma once
#include <SFML/Graphics.hpp>
#include "SettingsManager.hpp"
#include "SaveManager.hpp"
#include "UpgradeManager.hpp"
#include "ui/Container.hpp"
#include "ui/Text.hpp"
#include "ui/Button.hpp"
#include <vector>
#include <string>
#include <memory>

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС MENU
//
///////////////////////////////////////////////////////////////////////////

namespace UI {
    class Slider; // предварительное объявление
}

// Результаты игровой сессии
enum class SessionResult { None, Win, Lose };

class Menu {
public:
    // Состояния меню (экраны)
    enum class MenuState { Main, LevelSelect, Settings, Upgrades };

    // Информация об уровне для списка выбора
    struct LevelInfo {
        std::string filePath; // путь к файлу карты
        std::string name; // отображаемое название
        std::string id; // идентификатор (имя файла)
        int index = 0; // порядковый номер
    };

private:
    sf::RenderWindow& window;
    sf::View uiView;
    float uiScale = 1.0f;

    SettingsManager& settings;
    SaveManager& saveManager;
    UpgradeManager upgradeManager;

    MenuState state = MenuState::Main;
    std::vector<LevelInfo> levels;
    std::string selectedLevel;
    bool levelChosen = false;

    // Контейнеры экранов
    std::unique_ptr<UI::Container> mainContainer;
    std::unique_ptr<UI::Container> levelContainer;
    std::unique_ptr<UI::Container> settingsContainer;
    std::unique_ptr<UI::Container> upgradesContainer;
    std::unique_ptr<UI::Container> resultOverlay;

    // Указатели на виджеты для динамического обновления (без владения)
    UI::Container* cardsArea = nullptr;
    UI::Button* playBtnPtr = nullptr;
    UI::Slider* musicSliderPtr = nullptr;
    UI::Slider* sfxSliderPtr = nullptr;
    UI::Slider* sensSliderPtr = nullptr;
    UI::Slider* uiScaleSliderPtr = nullptr;
    UI::Button* fsBtnPtr = nullptr;
    UI::Button* vsyncBtnPtr = nullptr;
    UI::Container* headerContPtr = nullptr;
    UI::Container* btnsContPtr = nullptr;
    UI::Text* titleTextPtr = nullptr;
    UI::Text* moneyTextPtr = nullptr;
    
    // Списки указателей для меню улучшений
    std::vector<std::vector<UI::Text*>> upgradeValuePtrs;
    std::vector<std::vector<UI::Text*>> upgradeCostPtrs;
    std::vector<std::vector<UI::Button*>> upgradeBtnPtrs;

    // Временные настройки (до нажатия Сохранить)
    int tmpMusicVol = 100;
    int tmpSfxVol = 100;
    float tmpSensitivity = 1.0f;
    float tmpUiScale = 1.0f;
    bool tmpFullscreen = false;
    bool tmpVsync = true;
    bool windowRecreationRequired = false;

    SessionResult lastResult = SessionResult::None;
    std::string lastLevelPath;

    // Инициализация графического интерфейса
    void initUI();

    // Сканирование директории уровней
    void scanLevels();

    // Чтение метаданных уровня
    std::string readLevelName(const std::string& path) const;

    // Обновление выделения выбранного уровня
    void updateCardsSelection();

    // Синхронизация временных значений с менеджером настроек
    void syncSettingsToTmp();

    // Вспомогательный метод для создания подменю
    std::unique_ptr<UI::Container> createSubMenu(const std::string& title, UI::Container** outContent, UI::Container** outNav);

    // Создание меню глобальных улучшений
    std::unique_ptr<UI::Container> createUpgradeMenu();

public:
    // Конструктор инициализирует зависимости
    Menu(sf::RenderWindow& window, SettingsManager& settings, SaveManager& saveManager);

    // Обработка системных событий
    void handleEvents();

    // Отрисовка текущего экрана
    void render();

    // Проверка выбора уровня
    bool isLevelChosen() const;

    // Получение пути к выбранному уровню
    std::string getChosenLevel() const;

    // Сброс выбора
    void resetChoice();

    // Обновление размеров при изменении окна
    void updateViewSizes(sf::Vector2u windowSize);

    // Уведомление о завершении игры
    void notifyResult(SessionResult result, const std::string& levelPath);

    // Сброс результата сессии
    void resetLastResult();

    // Проверка необходимости пересоздания окна
    bool consumesWindowRecreationRequest();

    // Освобождение ресурсов
    void cleanup();

    // Получение баланса игрока
    int getMoney() const;

    // Получение доступа к менеджеру улучшений
    UpgradeManager& getUpgradeManager();
};
