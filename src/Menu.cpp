#include "Menu.hpp"
#include "ResourceManager.hpp"
#include "GameData.hpp"
#include "utils/FileReader.hpp"
#include "utils/Logger.hpp"
#include "ui/Slider.hpp"
#include "ui/Container.hpp"
#include "ui/Button.hpp"
#include "ui/Image.hpp"
#include "Colors.hpp"
#include <filesystem>
#include <algorithm>

#ifdef __ANDROID__
#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <SFML/System/NativeActivity.hpp>
#endif 

using Engine::Logger;

namespace fs = std::filesystem;

// Инициализирует системы и строит интерфейс
Menu::Menu(sf::RenderWindow& window, SettingsManager& settings, SaveManager& saveManager) 
    : window(window), settings(settings), saveManager(saveManager) {
    // загрузка улучшений из сохранений
    std::vector<UpgradeManager::TowerUpgrade> data;
    if (saveManager.getTowerData(data)) {
        upgradeManager.setAllUpgrades(data);
    } else {
        // инициализация базовыми значениями из GameData
        upgradeManager.initDefaults();
    }
    // установка callback для сохранения при изменении улучшений
    UpgradeManager* umPtr = &upgradeManager;
    SaveManager* smPtr = &saveManager;
    upgradeManager.setSaveCallback([umPtr, smPtr]() {
        smPtr->setTowerData(umPtr->getAllUpgrades());
        smPtr->save();
    });
    syncSettingsToTmp();
    scanLevels();
    initUI();
    updateViewSizes(window.getSize());
}

// Синхронизация временных значений с текущими настройками
void Menu::syncSettingsToTmp() {
    tmpMusicVol = settings.get<int>("music_volume", 100);
    tmpSfxVol = settings.get<int>("sfx_volume", 100);
    tmpSensitivity = settings.get<float>("sensitivity", 1.0f);
    tmpUiScale = settings.get<float>("ui_scale", 1.0f);
    tmpFullscreen = settings.get<bool>("fullscreen", false);
    tmpVsync = settings.get<bool>("vsync", true);

    // актуализация виджетов если они уже созданы
    if (musicSliderPtr) musicSliderPtr->setValue((float)tmpMusicVol);
    if (sfxSliderPtr) sfxSliderPtr->setValue((float)tmpSfxVol);
    if (sensSliderPtr) sensSliderPtr->setValue(tmpSensitivity);
    if (uiScaleSliderPtr) uiScaleSliderPtr->setValue(tmpUiScale);
    if (fsBtnPtr) fsBtnPtr->setText(tmpFullscreen ? "ВКЛ" : "ВЫКЛ");
    if (vsyncBtnPtr) vsyncBtnPtr->setText(tmpVsync ? "ВКЛ" : "ВЫКЛ");
}

// Построение иерархии контейнеров для каждого экрана
void Menu::initUI() {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());

    // главное меню
    mainContainer = std::make_unique<UI::Container>(winSize);
    mainContainer->setDirection(UI::Container::Direction::Column);
    mainContainer->setContentAlign(UI::Container::ContentAlign::Center);
    mainContainer->setItemAlign(UI::Container::ItemAlign::Center);
    mainContainer->setBackgroundTexture(ResourceManager::get("main-layer"), 128.f);
    mainContainer->setPadding({ 20.f, 20.f });
    mainContainer->setGap(30.f);

    auto headerCont = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 125.f));
    headerCont->setDirection(UI::Container::Direction::Column);
    headerCont->setContentAlign(UI::Container::ContentAlign::End);
    headerCont->setItemAlign(UI::Container::ItemAlign::Center);
    headerCont->setGap(10.f); 
    headerContPtr = headerCont.get();

    auto title = std::make_unique<UI::Text>(font, "PROJECT: Gyurza", 80, sf::Vector2f(winSize.x * 0.9f, 80.f));
    title->setAlignment(UI::Text::Align::Center);
    title->setColor(Colors::Theme::TextMain);
    titleTextPtr = title.get();
    headerCont->addChild(std::move(title));

    auto version = std::make_unique<UI::Text>(font, "v0.5a", 24);
    version->setAlignment(UI::Text::Align::Center);
    version->setColor(Colors::Theme::TextDark);
    headerCont->addChild(std::move(version));
    mainContainer->addChild(std::move(headerCont));

    auto btnsCont = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.8f, 320.f));
    btnsCont->setDirection(UI::Container::Direction::Column);
    btnsCont->setContentAlign(UI::Container::ContentAlign::Center);
    btnsCont->setItemAlign(UI::Container::ItemAlign::Center);
    btnsCont->setGap(15.f);
    btnsContPtr = btnsCont.get();

    // Задаем базовые размеры для сетки
    float btnW = 250.f;        // Базовая ширина кнопки
    float btnH = 64.f;         // Базовая высота кнопки
    float gap = 16.f;          // Отступ между плитками

    // Ширина длинной кнопки (две обычных ширины + отступ)
    float wideBtnW = btnW * 2 + gap;
    // Высота высокой кнопки (две обычных высоты + отступ)
    float tallBtnH = btnH * 2 + gap;

    // Обновляем помощник, чтобы он принимал sf::Vector2f size
    auto createMenuButton = [&](const sf::Texture& icon, sf::Vector2f iconScale, UI::Button::IconPlacement iconAlign,
        const std::string& label, unsigned int textSize, sf::Vector2f size, std::function<void()> onClick) {
        auto btn = std::make_unique<UI::Button>(icon, font, label, size, iconAlign);
        btn->setBackgroundTextures(
            &ResourceManager::get("button"),
            &ResourceManager::get("button-hover"),
            &ResourceManager::get("button-active"),
            &ResourceManager::get("button-disabled"),
            32.0f
        );
        btn->setIconScale(iconScale);
        btn->setTextSize(textSize);
        btn->setCallback(std::move(onClick));
        return btn;
        };

    // Верхняя широкая плитка: ИГРАТЬ
    btnsCont->addChild(createMenuButton(ResourceManager::get("icon-play"), { 1.f, 1.f }, UI::Button::IconPlacement::Right, "ИГРАТЬ", 48, sf::Vector2f(wideBtnW, tallBtnH), [this]() {
        state = MenuState::LevelSelect;
        }));

    // Горизонтальный контейнер для нижней части сетки
    auto bottomGrid = std::make_unique<UI::Container>(sf::Vector2f(wideBtnW, tallBtnH));
    bottomGrid->setDirection(UI::Container::Direction::Row);
    bottomGrid->setContentAlign(UI::Container::ContentAlign::Center);
    bottomGrid->setItemAlign(UI::Container::ItemAlign::Center);
    bottomGrid->setGap(gap);

    // Левая колонка сетки: НАСТРОЙКИ и ВЫХОД
    auto leftColumn = std::make_unique<UI::Container>(sf::Vector2f(btnW, tallBtnH));
    leftColumn->setDirection(UI::Container::Direction::Column);
    leftColumn->setContentAlign(UI::Container::ContentAlign::Center);
    leftColumn->setItemAlign(UI::Container::ItemAlign::Center);
    leftColumn->setGap(gap);

    leftColumn->addChild(createMenuButton(ResourceManager::get("icon-settings"), { 0.5f, 0.5f }, UI::Button::IconPlacement::Left, "НАСТРОЙКИ", 24, sf::Vector2f(btnW, btnH), [this]() {
        syncSettingsToTmp();
        state = MenuState::Settings;
        }));

    leftColumn->addChild(createMenuButton(ResourceManager::get("icon-exit"), { 0.5f, 0.5f }, UI::Button::IconPlacement::Left, "ВЫХОД", 24, sf::Vector2f(btnW, btnH), [this]() {
        window.close();
        }));

    bottomGrid->addChild(std::move(leftColumn));

    // Правая высокая плитка: УЛУЧШЕНИЯ
    bottomGrid->addChild(createMenuButton(ResourceManager::get("icon-upgrades"), { 0.75f, 0.75f }, UI::Button::IconPlacement::Top, "УЛУЧШЕНИЯ", 24, sf::Vector2f(btnW, tallBtnH), [this]() {
        state = MenuState::Upgrades;
    }));

    btnsCont->addChild(std::move(bottomGrid));
    mainContainer->addChild(std::move(btnsCont));

    // экран выбора уровня
    UI::Container* navArea = nullptr;
    levelContainer = createSubMenu("ВЫБОР УРОВНЯ", &cardsArea, &navArea);
    levelContainer->setBackgroundTexture(ResourceManager::get("panel"), 64.f);

    if (cardsArea) {
        cardsArea->setDirection(UI::Container::Direction::Row);
        cardsArea->setContentAlign(UI::Container::ContentAlign::Center);
        cardsArea-> setBackgroundTexture(ResourceManager::get("panel-light"), 64.f);
        cardsArea->setWrap(true);
        cardsArea->setGap(25.f);
        cardsArea->setPadding({ 20.f, 20.f });
        cardsArea->setScrollEnabled(true);

        for (const auto& level : levels) {
            sf::Vector2f cardSize(260.f, 140.f);
            auto card = std::make_unique<UI::Container>(cardSize);
            card->setDirection(UI::Container::Direction::Column);
            card->setContentAlign(UI::Container::ContentAlign::Start);
            card->setItemAlign(UI::Container::ItemAlign::Center);
            card->setBackgroundTexture(ResourceManager::get("card"), 12.f);

            auto numBlock = std::make_unique<UI::Container>(sf::Vector2f(cardSize.x, 45.f));
            numBlock->setDirection(UI::Container::Direction::Row);
            numBlock->setContentAlign(UI::Container::ContentAlign::Start);
            numBlock->setGap(20);
            numBlock->setItemAlign(UI::Container::ItemAlign::Center);
            auto icon = std::make_unique<UI::Image>(ResourceManager::get("icon-level"), sf::Vector2f(36.f, 36.f));
            numBlock->addChild(std::move(icon));
            auto numText = std::make_unique<UI::Text>(font, "УРОВЕНЬ " + std::to_string(level.index + 1), 26);
            numText->setColor(Colors::Theme::TextMain);
            numBlock->addChild(std::move(numText));
            card->addChild(std::move(numBlock));

            auto nameText = std::make_unique<UI::Text>(font, level.name, 20, sf::Vector2f(cardSize.x, cardSize.y - 55.f));
            nameText->setAlignment(UI::Text::Align::Center);
            nameText->setColor(Colors::Theme::TextMain);
            card->addChild(std::move(nameText));

            auto clicker = std::make_unique<UI::Button>(font, "", cardSize);
            clicker->setTransparent(true);
            clicker->setFollowsLayout(false);
            
            // обработка нажатия при отсутствии прокрутки
            clicker->setCallback([this, path = level.filePath, area = cardsArea]() {
                if (area && !area->isCurrentlyDragging()) {
                    selectedLevel = path;
                    updateCardsSelection();
                }
            });
            card->addChild(std::move(clicker));
            cardsArea->addChild(std::move(card));
        }
    }

    if (navArea) {
        auto startGameBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-play"), font, "ИГРАТЬ", sf::Vector2f(220.f, 60.f),  UI::Button::IconPlacement::Right);
        startGameBtn->setBackgroundTextures(
            &ResourceManager::get("button"),
            &ResourceManager::get("button-hover"),
            &ResourceManager::get("button-active"),
            &ResourceManager::get("button-disabled"),
            32.0f
        );
        startGameBtn->setIconScale({ 0.5f, 0.5f });
        startGameBtn->setCallback([this]() { if (!selectedLevel.empty()) levelChosen = true; });
        startGameBtn->setEnabled(false);
        playBtnPtr = startGameBtn.get();
        navArea->addChild(std::move(startGameBtn));
    }

    // экран настроек
    UI::Container* settingsContent = nullptr;
    UI::Container* settingsNav = nullptr;
    settingsContainer = createSubMenu("НАСТРОЙКИ", &settingsContent, &settingsNav);
    settingsContainer->setBackgroundTexture(ResourceManager::get("panel"), 64.f);

    if (settingsContent) {
        settingsContent->setGap(20.f);
        settingsContent->setPadding({ 20.f, 20.f });
        settingsContent->setItemAlign(UI::Container::ItemAlign::Center);
        settingsContent->setContentAlign(UI::Container::ContentAlign::Center);
        settingsContent->setBackgroundTexture(ResourceManager::get("panel-light"), 64.f);
        settingsContent->setScrollEnabled(true);

        // лямбда для создания строки меню с иконкой, заголовком и контролом
        auto createRow = [&](const sf::Texture& iconTex, const std::string& label, std::unique_ptr<UI::Widget> control, std::unique_ptr<UI::Text> valueText = nullptr) {
            auto row = std::make_unique<UI::Container>(sf::Vector2f(950.f, 60.f));
            row->setDirection(UI::Container::Direction::Row);
            row->setContentAlign(UI::Container::ContentAlign::Center);
            row->setItemAlign(UI::Container::ItemAlign::Center);
            row->setGap(10.f);

            // Добавление иконки настройки
            auto icon = std::make_unique<UI::Image>(iconTex, sf::Vector2f(48.f, 48.f));
            row->addChild(std::move(icon));
            // Добавление названия настройки
            auto text = std::make_unique<UI::Text>(font, label, 24, sf::Vector2f(350.f, 60.f));
            text->setColor(Colors::Theme::TextMain);
            row->addChild(std::move(text));

            row->addChild(std::move(control));
            if (valueText) {
                row->addChild(std::move(valueText));
            }
            return row;
        };

        // Настройка: Громкость музыки
        auto musicSlider = std::make_unique<UI::Slider>(font, 0.f, 100.f, (float)tmpMusicVol, sf::Vector2f(350.f, 30.f));
        musicSliderPtr = musicSlider.get();
        musicSlider->setCallback([this](float value) { tmpMusicVol = (int)value; });
        settingsContent->addChild(createRow(ResourceManager::get("icon-music"), "ГРОМКОСТЬ МУЗЫКИ", std::move(musicSlider)));

        // Настройка: Громкость звуков
        auto sfxSlider = std::make_unique<UI::Slider>(font, 0.f, 100.f, (float)tmpSfxVol, sf::Vector2f(350.f, 30.f));
        sfxSliderPtr = sfxSlider.get();
        sfxSlider->setCallback([this](float value) { tmpSfxVol = (int)value; });
        settingsContent->addChild(createRow(ResourceManager::get("icon-audio"), "ГРОМКОСТЬ ЗВУКОВ", std::move(sfxSlider)));

        // Настройка: Чувствительность
        const sf::Texture& sensIcon = ResourceManager::get("icon-sensivity");
        auto sensSlider = std::make_unique<UI::Slider>(font, 0.5f, 3.0f, tmpSensitivity, sf::Vector2f(350.f, 30.f));
        sensSlider->setPrecision(1);
        sensSliderPtr = sensSlider.get();
        sensSlider->setCallback([this](float value) { tmpSensitivity = value; });
        settingsContent->addChild(createRow(sensIcon, "ЧУВСТВИТЕЛЬНОСТЬ", std::move(sensSlider)));

        // Настройка: Масштаб интерфейса
#ifdef __ANDROID__
        float minScale = 0.9f;
        float maxScale = 1.4f;
#else
        float minScale = 0.6f;
        float maxScale = 1.6f;
#endif
        auto uiScaleSlider = std::make_unique<UI::Slider>(font, minScale, maxScale, tmpUiScale, sf::Vector2f(350.f, 30.f));
        uiScaleSlider->setPrecision(1);
        uiScaleSliderPtr = uiScaleSlider.get();
        uiScaleSlider->setCallback([this](float value) { tmpUiScale = value; });
        settingsContent->addChild(createRow(ResourceManager::get("icon-display"), "МАСШТАБ ИНТЕРФЕЙСА", std::move(uiScaleSlider)));

#ifndef __ANDROID__ 
        // Настройка: Вертикальная синхронизация
        std::string vsLabel = tmpVsync ? "ВКЛ" : "ВЫКЛ";
        auto vsyncBtn = std::make_unique<UI::Button>(font, vsLabel, sf::Vector2f(350.f, 45.f));
        vsyncBtnPtr = vsyncBtn.get();
        vsyncBtn->setBackgroundTextures(
            &ResourceManager::get("card"),
            &ResourceManager::get("card-light"),
            &ResourceManager::get("card-dark"),
            nullptr,
            12.0f
        );
        vsyncBtn->setCallback([this]() {
            tmpVsync = !tmpVsync;
            if (vsyncBtnPtr) vsyncBtnPtr->setText(tmpVsync ? "ВКЛ" : "ВЫКЛ");
        });
        settingsContent->addChild(createRow(ResourceManager::get("icon-vsync"), "ВЕРТИКАЛЬНАЯ СИНХР.", std::move(vsyncBtn)));

        // Настройка: Полноэкранный режим
        std::string fsLabel = tmpFullscreen ? "ВКЛ" : "ВЫКЛ";
        auto fsBtn = std::make_unique<UI::Button>(font, fsLabel, sf::Vector2f(350.f, 45.f));
        fsBtn->setBackgroundTextures(
            &ResourceManager::get("card"),
            &ResourceManager::get("card-light"),
            &ResourceManager::get("card-dark"),
            nullptr,
            32.0f
        );
        fsBtnPtr = fsBtn.get();
        fsBtn->setCallback([this]() {
            tmpFullscreen = !tmpFullscreen;
            if (fsBtnPtr) fsBtnPtr->setText(tmpFullscreen ? "ВКЛ" : "ВЫКЛ");
        });
        
        settingsContent->addChild(createRow(ResourceManager::get("icon-fullscreen"), "ПОЛНОЭКРАННЫЙ РЕЖИМ", std::move(fsBtn)));
#endif
    }
    
    if (settingsNav) {
        auto saveBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-save"), font, "СОХРАНИТЬ", sf::Vector2f(220.f, 60.f), UI::Button::IconPlacement::Right);
        saveBtn->setBackgroundTextures(
            &ResourceManager::get("button"),
            &ResourceManager::get("button-hover"),
            &ResourceManager::get("button-active"),
            &ResourceManager::get("button-disabled"),
            32.0f
        );
        saveBtn->setIconScale({ 0.5f, 0.5f });
        saveBtn->setTextSize(20);
        saveBtn->setCallback([this]() {
            bool oldFs = settings.get<bool>("fullscreen", false);
            settings.set<int>("music_volume", tmpMusicVol);
            settings.set<int>("sfx_volume", tmpSfxVol);
            settings.set<float>("sensitivity", tmpSensitivity);
            settings.set<float>("ui_scale", tmpUiScale);
            settings.set<bool>("fullscreen", tmpFullscreen);
            settings.set<bool>("vsync", tmpVsync);
            settings.save();
            
            // Применение вертикальной синхронизации
            window.setVerticalSyncEnabled(tmpVsync);

            updateViewSizes(window.getSize());
            window.setView(uiView);

            if (oldFs != tmpFullscreen) {
                windowRecreationRequired = true;
            }
            state = MenuState::Main;
        });
        settingsNav->addChild(std::move(saveBtn));
    }

    upgradesContainer = createUpgradeMenu();

    resultOverlay = std::make_unique<UI::Container>(winSize);
    resultOverlay->setDirection(UI::Container::Direction::Column);
    resultOverlay->setContentAlign(UI::Container::ContentAlign::Center);
    resultOverlay->setItemAlign(UI::Container::ItemAlign::Center);
    resultOverlay->setBackgroundColor(Colors::Theme::Overlay);
    resultOverlay->setDrawBackground(false);
    resultOverlay->setBackgroundTexture(ResourceManager::get("panel-light"), 64.f);
}

// Создание вложенного меню
std::unique_ptr<UI::Container> Menu::createSubMenu(const std::string& title, UI::Container** outContent, UI::Container** outNav) {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());

    auto root = std::make_unique<UI::Container>(winSize);
    root->setDirection(UI::Container::Direction::Column);
    root->setContentAlign(UI::Container::ContentAlign::Center); 
    root->setItemAlign(UI::Container::ItemAlign::Center);
    root->setBackgroundTexture(ResourceManager::get("panel"), 64.f);
    root->setPadding({ 20.f, 5.f });
    root->setGap(20.f);

    auto header = std::make_unique<UI::Text>(font, title, 60, sf::Vector2f(winSize.x * 0.9f, 80.f));
    header->setAlignment(UI::Text::Align::Center);
    header->setColor(Colors::Theme::TextMain);
    root->addChild(std::move(header));

    auto content = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 400.f)); 
    content->setDirection(UI::Container::Direction::Column);
    content->setContentAlign(UI::Container::ContentAlign::Center); 
    content->setItemAlign(UI::Container::ItemAlign::Center);
    content->setBackgroundTexture(ResourceManager::get("panel-light"), 64.f);
    if (outContent) *outContent = content.get();
    root->addChild(std::move(content));

    auto nav = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 80.f));
    nav->setDirection(UI::Container::Direction::Row);
    nav->setContentAlign(UI::Container::ContentAlign::Center);
    nav->setGap(30.f);
    auto back = std::make_unique<UI::Button>(ResourceManager::get("icon-back"), font, "НАЗАД", sf::Vector2f(220.f, 60.f), UI::Button::IconPlacement::Left);
    back->setBackgroundTextures(
        &ResourceManager::get("button"),
        &ResourceManager::get("button-hover"),
        &ResourceManager::get("button-active"),
        &ResourceManager::get("button-disabled"),
        32.0f
    );
    back->setIconScale({ 0.5f, 0.5f });
    back->setTextSize(20);
    back->setCallback([this]() { state = MenuState::Main; selectedLevel = ""; updateCardsSelection(); });
    nav->addChild(std::move(back));
    if (outNav) *outNav = nav.get();
    root->addChild(std::move(nav));

    return root;
}

// Создание подменю глобальных улучшений
std::unique_ptr<UI::Container> Menu::createUpgradeMenu() {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());

    float headerHeight = 80.f;
    float currencyHeight = 75.f;
    float navHeight = 80.f;
    float contentHeight = winSize.y - headerHeight - currencyHeight - navHeight - 60.f;

    auto root = std::make_unique<UI::Container>(winSize);
    root->setDirection(UI::Container::Direction::Column);
    root->setContentAlign(UI::Container::ContentAlign::Center);
    root->setItemAlign(UI::Container::ItemAlign::Center);
    root->setBackgroundTexture(ResourceManager::get("panel"), 64.f);
    root->setPadding({ 20.f, 5.f });
    root->setGap(10.f);

    // header
    auto header = std::make_unique<UI::Text>(font, "УЛУЧШЕНИЯ", 60, sf::Vector2f(winSize.x * 0.9f, headerHeight));
    header->setAlignment(UI::Text::Align::Center);
    header->setColor(Colors::Theme::TextMain);
    root->addChild(std::move(header));

    auto content = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 400.f));
    content->setDirection(UI::Container::Direction::Column);
    content->setContentAlign(UI::Container::ContentAlign::Center);
    content->setItemAlign(UI::Container::ItemAlign::Center);
    content->setBackgroundTexture(ResourceManager::get("panel-light"), 64.f);
    content->setGap(10.f);

    // currency
    auto currency = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, currencyHeight));
    currency->setDirection(UI::Container::Direction::Row);
    currency->setContentAlign(UI::Container::ContentAlign::Center);
    currency->setItemAlign(UI::Container::ItemAlign::Center);
    currency->setGap(15.f);
    currency->setDrawBackground(true);
    currency->setBackgroundColor(sf::Color::Transparent);

    auto coinIcon = std::make_unique<UI::Image>(ResourceManager::get("icon-money"), sf::Vector2f(48.f, 48.f));
    currency->addChild(std::move(coinIcon));

    // текст валюты (глобальные деньги) - показываем из SaveManager
    auto moneyText = std::make_unique<UI::Text>(font, std::to_string(saveManager.getMoney()), 24);
    moneyTextPtr = moneyText.get();
    currency->addChild(std::move(moneyText));

    content->addChild(std::move(currency));

    // cards - scroll container
    auto cards = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, contentHeight));
    cards->setDirection(UI::Container::Direction::Row);
    cards->setContentAlign(UI::Container::ContentAlign::Center);
    cards->setItemAlign(UI::Container::ItemAlign::Center);
    cards->setWrap(true);
    cards->setGap(15.f);
    cards->setDrawBackground(true);
    cards->setBackgroundColor(sf::Color::Transparent);
    cards->setScrollEnabled(true);

    std::vector<std::string> towerTypes = GameData::getTowerNames();
    // порядок характеристик: ранг, урон, скорострельность, дальность, уровень
    std::vector<std::string> statNames = { "РАНГ", "АТАКА", "СКОР. АТАКИ", "РАДИУС", "УРОВЕНЬ" };

    std::vector<std::string> towerDisplayNames;
    for (const auto& name : towerTypes) {
        std::string upper = name;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        towerDisplayNames.push_back(upper);
    }

    upgradeValuePtrs.clear();
    upgradeValuePtrs.resize(towerTypes.size());
    upgradeCostPtrs.clear();
    upgradeCostPtrs.resize(towerTypes.size());
    upgradeBtnPtrs.clear();
    upgradeBtnPtrs.resize(towerTypes.size());

    for (size_t t = 0; t < towerTypes.size(); ++t) {
        auto towerCard = std::make_unique<UI::Container>(sf::Vector2f(450.f, 260.f));
        towerCard->setDirection(UI::Container::Direction::Column);
        towerCard->setContentAlign(UI::Container::ContentAlign::Center);
        towerCard->setItemAlign(UI::Container::ItemAlign::Center);
        towerCard->setPadding({ 10.f, 8.f });
        towerCard->setGap(5.f);
        towerCard->setBackgroundTexture(ResourceManager::get("card"), 12.f);

        auto towerName = std::make_unique<UI::Text>(font, towerDisplayNames[t], 28);
        towerName->setColor(Colors::Theme::TextMain);
        towerCard->addChild(std::move(towerName));

        for (size_t s = 0; s < statNames.size(); ++s) {
            auto statRow = std::make_unique<UI::Container>(sf::Vector2f(400.f, 36.f));
            statRow->setDirection(UI::Container::Direction::Row);
            statRow->setContentAlign(UI::Container::ContentAlign::Center);
            statRow->setItemAlign(UI::Container::ItemAlign::Center);
            statRow->setGap(5.f);

            auto statNameText = std::make_unique<UI::Text>(font, statNames[s], 18, sf::Vector2f(170.f , 32.f));
            statNameText->setAlignment(UI::Text::Align::Left);
            statNameText->setColor(Colors::Theme::TextMain);
            statRow->addChild(std::move(statNameText));

            // получение актуального значения
            float currentValue = 0.f;
            int currentStep = 0;
            std::string statKey = (s == 0 ? "rank" : (s == 1 ? "damage" : (s == 2 ? "firerate" : (s == 3 ? "range" : "level"))));

            if (s == 0) currentStep = upgradeManager.getRank(towerTypes[t]);
            else if (s == 1) currentValue = upgradeManager.getDamage(towerTypes[t]);
            else if (s == 2) currentValue = upgradeManager.getFirerate(towerTypes[t]);
            else if (s == 3) currentValue = upgradeManager.getRange(towerTypes[t]);
            else if (s == 4) currentStep = upgradeManager.getLevel(towerTypes[t]);

            std::string valueStr;
            if (s == 0 || s == 4) valueStr = std::to_string(currentStep);
            else if (s == 2) {
                valueStr = std::to_string(currentValue);
                valueStr = valueStr.substr(0, valueStr.find('.') + 2);
            } else valueStr = std::to_string((int)currentValue);

            auto statValue = std::make_unique<UI::Text>(font, valueStr, 18, sf::Vector2f(70.f, 32.f));
            statValue->setAlignment(UI::Text::Align::Left);
            statValue->setColor(Colors::Theme::TextMain);
            upgradeValuePtrs[t].push_back(statValue.get());
            statRow->addChild(std::move(statValue));

            int currentCost = upgradeManager.getUpgradeCost(towerTypes[t], (int)s);
            auto costText = std::make_unique<UI::Text>(font, std::to_string(currentCost), 18, sf::Vector2f(80.f, 32.f));
            costText->setAlignment(UI::Text::Align::Left);
            costText->setColor(Colors::Theme::TextYellow);
            upgradeCostPtrs[t].push_back(costText.get());
            statRow->addChild(std::move(costText));
                        
            auto upgradeBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-upgrade2"), sf::Vector2f(50.f, 32.f));
            upgradeBtn->setIconScale({ 32.f / 96.f, 32.f / 96.f });
            upgradeBtn->setTextSize(20);
            
            auto& towerTypeCopy = towerTypes[t];
            UpgradeManager* umPtr = &upgradeManager;
            SaveManager* smPtr = &saveManager;

            upgradeBtn->setCallback([umPtr, smPtr, towerTypeCopy, statIndex = s, statKey]() {
                if (umPtr->isStatAtLimit(towerTypeCopy, statKey)) return;
                
                int cost = umPtr->getUpgradeCost(towerTypeCopy, (int)statIndex);
                if (smPtr->spendMoney(cost)) {
                    if (statIndex == 0) umPtr->upgradeRank(towerTypeCopy);
                    else if (statIndex == 1) umPtr->upgradeDamage(towerTypeCopy, 0.1f);
                    else if (statIndex == 2) umPtr->upgradeFirerate(towerTypeCopy, 0.1f);
                    else if (statIndex == 3) umPtr->upgradeRange(towerTypeCopy, 0.1f);
                    else if (statIndex == 4) umPtr->upgradeMaxLevel(towerTypeCopy);
                }
            });

            upgradeBtnPtrs[t].push_back(upgradeBtn.get());
            statRow->addChild(std::move(upgradeBtn));
            towerCard->addChild(std::move(statRow));
        }
        cards->addChild(std::move(towerCard));
    }
    content->addChild(std::move(cards));
    root->addChild(std::move(content));

    // nav
    auto nav = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, navHeight));
    nav->setDirection(UI::Container::Direction::Row);
    nav->setContentAlign(UI::Container::ContentAlign::Center);
    nav->setItemAlign(UI::Container::ItemAlign::Center);
    nav->setGap(30.f);
    nav->setDrawBackground(true);
    nav->setBackgroundColor(sf::Color::Transparent);

    auto back = std::make_unique<UI::Button>(ResourceManager::get("icon-back"), font, "НАЗАД", sf::Vector2f(220.f, 60.f), UI::Button::IconPlacement::Left);
    back->setBackgroundTextures(
        &ResourceManager::get("button"),
        &ResourceManager::get("button-hover"),
        &ResourceManager::get("button-active"),
        &ResourceManager::get("button-disabled"),
        32.0f
    );
    back->setIconScale({ 0.5f, 0.5f });
    back->setTextSize(20);
    back->setCallback([this]() { state = MenuState::Main; });
    nav->addChild(std::move(back));
    root->addChild(std::move(nav));

    return root;
}

// Обновление состояния выбора карточек уровней
void Menu::updateCardsSelection() {
    if (!cardsArea) return;
    for (size_t i = 0; i < cardsArea->getChildrenCount(); ++i) {
        auto* card = static_cast<UI::Container*>(cardsArea->getChild(i));
        if (i >= levels.size()) continue;
        const auto& level = levels[i];
        if (level.filePath == selectedLevel) {
            card->setBackgroundTexture(ResourceManager::get("card-light"), 12.f);
        } else {
            card->setBackgroundTexture(ResourceManager::get("card"), 12.f);
        }
    }
    if (playBtnPtr) playBtnPtr->setEnabled(!selectedLevel.empty());
}

// Обработка событий окна
void Menu::handleEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) window.close();
        if (const auto* resized = event->getIf<sf::Event::Resized>()) updateViewSizes(resized->size);
        UI::Container* current = nullptr; 
        switch (state) {
            case MenuState::Main: current = mainContainer.get(); break;
            case MenuState::LevelSelect: current = levelContainer.get(); break;
            case MenuState::Settings: current = settingsContainer.get(); break;
            case MenuState::Upgrades: current = upgradesContainer.get(); break;
        }
        if (current) current->handleEvent(*event, window, uiView);
    }
}

// Отрисовка интерфейса меню
void Menu::render() {
    window.clear(Colors::Theme::Background);
    window.setView(uiView);
    UI::Container* current = nullptr;
    switch (state) {
        case MenuState::Main: current = mainContainer.get(); break;
        case MenuState::LevelSelect: current = levelContainer.get(); break;
        case MenuState::Settings: current = settingsContainer.get(); break;
        case MenuState::Upgrades: current = upgradesContainer.get(); break;
    }
    if (current) current->render(window);
    
    // обновление текста валюты в меню улучшений
    if (state == MenuState::Upgrades && moneyTextPtr) {
        moneyTextPtr->setText(std::to_string(saveManager.getMoney()));
        
        std::vector<std::string> towerTypes = GameData::getTowerNames();
        for (size_t t = 0; t < upgradeValuePtrs.size() && t < towerTypes.size(); ++t) {
            for (size_t s = 0; s < upgradeValuePtrs[t].size(); ++s) {
                float currentValue = 0.f;
                int currentStep = 0;
                std::string statKey = (s == 0 ? "rank" : (s == 1 ? "damage" : (s == 2 ? "firerate" : (s == 3 ? "range" : "level"))));

                if (s == 0) currentStep = upgradeManager.getRank(towerTypes[t]);
                else if (s == 1) currentValue = upgradeManager.getDamage(towerTypes[t]);
                else if (s == 2) currentValue = upgradeManager.getFirerate(towerTypes[t]);
                else if (s == 3) currentValue = upgradeManager.getRange(towerTypes[t]);
                else if (s == 4) currentStep = upgradeManager.getLevel(towerTypes[t]);
                
                std::string valueStr;
                if (s == 0 || s == 4) valueStr = std::to_string(currentStep);
                else if (s == 2) {
                    valueStr = std::to_string(currentValue);
                    valueStr = valueStr.substr(0, valueStr.find('.') + 2);
                } else valueStr = std::to_string((int)currentValue);
                
                upgradeValuePtrs[t][s]->setText(valueStr);
                
                int cost = upgradeManager.getUpgradeCost(towerTypes[t], (int)s);
                bool atLimit = upgradeManager.isStatAtLimit(towerTypes[t], statKey);

                if (upgradeCostPtrs[t].size() > s && upgradeCostPtrs[t][s]) {
                    upgradeCostPtrs[t][s]->setText(atLimit ? "MAX" : std::to_string(cost));
                    
                    if (atLimit) {
                        upgradeCostPtrs[t][s]->setColor(sf::Color(150, 150, 150));
                    } else if (saveManager.getMoney() < cost) {
                        upgradeCostPtrs[t][s]->setColor(Colors::Theme::TextRed);
                    } else {
                        upgradeCostPtrs[t][s]->setColor(Colors::Theme::TextGreen);
                    }
                }
                if (upgradeBtnPtrs[t].size() > s && upgradeBtnPtrs[t][s]) {
                    upgradeBtnPtrs[t][s]->setEnabled(!atLimit);
                }
            }
        }
    }
    
    if (lastResult != SessionResult::None) resultOverlay->render(window);
    window.display();
}

// Обновление размеров элементов интерфейса при изменении окна
void Menu::updateViewSizes(sf::Vector2u windowSize) {
    float sw = static_cast<float>(windowSize.x);
    float sh = static_cast<float>(windowSize.y);

    float baseScale = sh / 1080.f;

    uiScale = baseScale * tmpUiScale;
    if (uiScale <= 0.1f) uiScale = 1.0f;

    float uiH = sh / uiScale;
    float uiW = uiH * (sw / sh);
    uiView = sf::View(sf::FloatRect({ 0.f, 0.f }, { uiW, uiH }));

    sf::Vector2f rootSize(uiW * 0.9f, uiH * 0.95f);
    sf::Vector2f rootPos((uiW - rootSize.x) / 2.f, (uiH - rootSize.y) / 2.f);

    if (mainContainer) {
        if (headerContPtr) headerContPtr->setSize(sf::Vector2f(rootSize.x * 0.9f, 125.f));
        if (btnsContPtr) btnsContPtr->setSize(sf::Vector2f(rootSize.x * 0.6f, 320.f));
        if (titleTextPtr) titleTextPtr->setMaxWidth(rootSize.x * 0.8f);
        mainContainer->setSize(rootSize);
        mainContainer->setPosition(rootPos);
        mainContainer->rebuild(); 
    }

    if (upgradesContainer) {
        auto towerTypes = GameData::getTowerNames();
        for (size_t t = 0; t < upgradeValuePtrs.size() && t < towerTypes.size(); ++t) {
            for (size_t s = 0; s < upgradeValuePtrs[t].size(); ++s) {
                if (upgradeValuePtrs[t][s]) upgradeValuePtrs[t][s]->setSize(sf::Vector2f(70.f, 35.f));
            }
        }
        for (size_t t = 0; t < upgradeCostPtrs.size() && t < towerTypes.size(); ++t) {
            for (size_t s = 0; s < upgradeCostPtrs[t].size(); ++s) {
                if (upgradeCostPtrs[t][s]) upgradeCostPtrs[t][s]->setSize(sf::Vector2f(80.f, 35.f));
            }
        }
        upgradesContainer->rebuild();
    }

    auto updateSub = [&](std::unique_ptr<UI::Container>& cont) {
        if (cont) {
            float headerH = 80.f;
            float navH = 80.f;
            float gap = 20.f;
            float p = 5.f;
            float contentH = rootSize.y - headerH - navH - gap * 2.f - p * 2.f;

            for (size_t i = 0; i < cont->getChildrenCount(); ++i) {
                auto* child = cont->getChild(i); 
                if (!child) continue;
                float targetWidth = rootSize.x * 0.95f;

                if (cont == upgradesContainer && i == 1) {
                    child->setSize(sf::Vector2f(targetWidth, contentH));
                    if (auto* asContainer = dynamic_cast<UI::Container*>(child)) {
                        if (asContainer->getChildrenCount() >= 2) {
                            auto* currency = asContainer->getChild(0);
                            auto* cards = asContainer->getChild(1);
                            if (currency) {
                                currency->setSize(sf::Vector2f(targetWidth, currency->getSize().y));
                            }
                            if (cards) {
                                cards->setSize(sf::Vector2f(targetWidth, child->getSize().y - (currency ? currency->getSize().y : 0.f) - 10.f));
                            }
                        }
                    }
                }
                else if (i == 1) {
                    child->setSize(sf::Vector2f(targetWidth, contentH));
                }
                else {
                    child->setSize(sf::Vector2f(targetWidth, child->getSize().y));
                    if (auto* asText = dynamic_cast<UI::Text*>(child)) asText->setMaxWidth(targetWidth);
                }
            }
            cont->setSize(rootSize);
            cont->setPosition(rootPos);
            cont->rebuild();
        }
    };

    updateSub(levelContainer);
    updateSub(settingsContainer);
    updateSub(upgradesContainer);

    if (resultOverlay) {
        resultOverlay->setSize(rootSize);
        resultOverlay->setPosition(rootPos);
        resultOverlay->rebuild();
    }
}

// Поиск файлов уровней в директориях
void Menu::scanLevels() {
    levels.clear();

#ifdef __ANDROID__
    ANativeActivity* activity = sf::getNativeActivity();
    AAssetDir* assetDir = AAssetManager_openDir(activity->assetManager, "levels");
    const char* fileName = nullptr;

    while ((fileName = AAssetDir_getNextFileName(assetDir)) != nullptr) {
        std::string sName = fileName; 
        if (sName.size() > 4 && sName.substr(sName.size() - 4) == ".map") {
            LevelInfo info;
            info.filePath = "levels/" + sName;
            info.id = sName.substr(0, sName.find_last_of('.'));
            info.name = readLevelName(info.filePath);
            levels.push_back(info);
        }
    }
    AAssetDir_close(assetDir);

    std::sort(levels.begin(), levels.end(), [](const LevelInfo& a, const LevelInfo& b) {
        return a.id < b.id;
    });

    for (int i = 0; i < (int)levels.size(); i++) levels[i].index = i;

#else
    const std::string dirPath = "data/levels/";

    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        Logger::error("[Menu]: Папка уровней не найдена: {}", dirPath);
        return;
    }

    std::vector<fs::path> mapPaths;
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".map") {
            mapPaths.push_back(entry.path());
        }
    }
    
    std::sort(mapPaths.begin(), mapPaths.end());

    for (int i = 0; i < (int)mapPaths.size(); ++i) {
        LevelInfo info;
        info.filePath = mapPaths[i].string();
        info.id = mapPaths[i].stem().string(); 
        info.name = readLevelName(info.filePath);
        info.index = i;
        levels.push_back(info);
    }
#endif

    Logger::debug("Успешно просканировано {} уровней", (int)levels.size());
}

// Чтение названия уровня из файла
std::string Menu::readLevelName(const std::string& path) const {
    auto content = readFile(path);
    if (!content || content->empty()) return "Безымянный";
    std::string search = "name=";
    size_t pos = content->find(search);
    if (pos == std::string::npos) return "Безымянный";
    size_t start = pos + search.length();
    size_t end = content->find("\n", start);
    std::string name = (end == std::string::npos) ? content->substr(start) : content->substr(start, end - start);
    if (!name.empty() && name.back() == '\r') name.pop_back();
    return name.empty() ? "Безымянный" : name;
}

// Получение состояния выбора уровня
bool Menu::isLevelChosen() const { return levelChosen; }

// Получение пути выбранного уровня
std::string Menu::getChosenLevel() const { return selectedLevel; }

// Сброс выбора уровня
void Menu::resetChoice() { levelChosen = false; selectedLevel = ""; updateCardsSelection(); }

// Сброс последнего результата
void Menu::resetLastResult() {
    lastResult = SessionResult::None;
}

// Проверка необходимости пересоздания окна
bool Menu::consumesWindowRecreationRequest() {
    bool req = windowRecreationRequired; 
    windowRecreationRequired = false; 
    return req; 
}

// Уведомление о результате сессии
void Menu::notifyResult(SessionResult result, const std::string& levelPath) {
    lastResult = result;
    lastLevelPath = levelPath;
    resultOverlay->clearChildren();
    auto& font = ResourceManager::getFont("main");
    std::string msg = (result == SessionResult::Win) ? "ПОБЕДА!" : "ПОРАЖЕНИЕ";
    auto text = std::make_unique<UI::Text>(font, msg, 80);
    text->setColor((result == SessionResult::Win) ? Colors::Palette::PastelRed : Colors::Palette::PastelGreen);
    resultOverlay->addChild(std::move(text));
    auto back = std::make_unique<UI::Button>(font, "В МЕНЮ", sf::Vector2f(200.f, 60.f));
    back->setCallback([this]() { lastResult = SessionResult::None; });
    resultOverlay->addChild(std::move(back));
}

// Очистка ресурсов интерфейса
void Menu::cleanup() {
    if (mainContainer) mainContainer.reset();
    if (levelContainer) levelContainer.reset();
    if (settingsContainer) settingsContainer.reset();
    if (upgradesContainer) upgradesContainer.reset();
    if (resultOverlay) resultOverlay.reset();

    // Очистка векторов указателей
    upgradeValuePtrs.clear();
    upgradeCostPtrs.clear();
    upgradeBtnPtrs.clear();
    moneyTextPtr = nullptr;
}

// Получение количества денег
int Menu::getMoney() const {
    return saveManager.getMoney();
}

// Получение менеджера улучшений
UpgradeManager& Menu::getUpgradeManager() {
    return upgradeManager;
}