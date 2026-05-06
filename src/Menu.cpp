#include "Menu.hpp"
#include "ResourceManager.hpp"
#include "GameData.hpp"
#include "utils/FileReader.hpp"
#include "utils/Logger.hpp"
#include "ui/Slider.hpp"
#include "ui/Container.hpp"
#include "ui/Button.hpp"
#include "ui/Image.hpp"
#include "ui/Toggle.hpp"
#include "Colors.hpp"
#include "Version.hpp"
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
    
    // Загрузка всех данных улучшений (башни + мета) из одного JSON-объекта
    json allUpgradeData;
    if (saveManager.getUpgradeData(allUpgradeData)) {
        if (allUpgradeData.contains("towers")) {
            upgradeManager.setAllUpgrades(allUpgradeData["towers"].get<std::vector<UpgradeManager::TowerUpgrade>>());
        }
        if (allUpgradeData.contains("meta")) {
            upgradeManager.setAllMetaUpgrades(allUpgradeData["meta"].get<std::vector<UpgradeManager::MetaUpgrade>>());
        }
    } else {
        upgradeManager.initDefaults();
    }

    UpgradeManager* umPtr = &upgradeManager;
    SaveManager* smPtr = &saveManager;
    upgradeManager.setSaveCallback([umPtr, smPtr]() {
        json j;
        j["towers"] = umPtr->getAllUpgrades();
        j["meta"] = umPtr->getAllMetaUpgrades();
        smPtr->setUpgradeData(j);
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
    tmpVfxHit = settings.get<bool>("vfx_hit", true);
    tmpVfxTrail = settings.get<bool>("vfx_trail", true);
    tmpVfxDeath = settings.get<bool>("vfx_death", true);
    tmpAnimation = settings.get<bool>("animation", true);

    if (musicSliderPtr) musicSliderPtr->setValue((float)tmpMusicVol);
    if (sfxSliderPtr) sfxSliderPtr->setValue((float)tmpSfxVol);
    if (sensSliderPtr) sensSliderPtr->setValue(tmpSensitivity);
    if (uiScaleSliderPtr) uiScaleSliderPtr->setValue(tmpUiScale);
    if (fsBtnPtr) fsBtnPtr->setText(tmpFullscreen ? "ВКЛ" : "ВЫКЛ");
    if (vsyncBtnPtr) vsyncBtnPtr->setText(tmpVsync ? "ВКЛ" : "ВЫКЛ");
}

// Построение иерархии контейнеров
void Menu::initUI() {
    mainContainer.reset();
    settingsContainer.reset();
    levelContainer.reset();
    upgradesContainer.reset();

    cardsArea = nullptr;
    playBtnPtr = nullptr;
    musicSliderPtr = nullptr;
    sfxSliderPtr = nullptr;
    sensSliderPtr = nullptr;
    uiScaleSliderPtr = nullptr;
    fsBtnPtr = nullptr;
    vsyncBtnPtr = nullptr;
    headerContPtr = nullptr;
    btnsContPtr = nullptr;
    titleTextPtr = nullptr;
    moneyTextPtr = nullptr;
    totalStarsTextPtr = nullptr;

    mainContainer = createMainMenu();
    levelContainer = createLevelSelectMenu();
    settingsContainer = createSettingsMenu();
    upgradesContainer = createUpgradeMenu();

    sf::Vector2f winSize = sf::Vector2f(window.getSize());
    resultOverlay = std::make_unique<UI::Container>(winSize);
    resultOverlay->setBackgroundColor(Colors::Theme::Overlay);
}

std::unique_ptr<UI::Container> Menu::createMainMenu() {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());

    auto root = std::make_unique<UI::Container>(winSize);
    root->setDirection(UI::Container::Direction::Column);
    root->setContentAlign(UI::Container::ContentAlign::Center);
    root->setItemAlign(UI::Container::ItemAlign::Center);
    root->setBackgroundTexture(ResourceManager::get("main-layer"), 128.f);                                                                                     
    root->setPadding({ 0.f, 20.f });
    root->setGap(30.f);

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

    auto version = std::make_unique<UI::Text>(font, std::string(GAME_VERSION), 24);
    version->setAlignment(UI::Text::Align::Center);
    version->setColor(Colors::Theme::TextDark);
    headerCont->addChild(std::move(version));
    root->addChild(std::move(headerCont));

    auto btnsCont = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.8f, 320.f));
    btnsCont->setDirection(UI::Container::Direction::Column);
    btnsCont->setContentAlign(UI::Container::ContentAlign::Center);
    btnsCont->setItemAlign(UI::Container::ItemAlign::Center);
    btnsCont->setGap(15.f);
    btnsContPtr = btnsCont.get();

    float btnW = 250.f, btnH = 64.f, gap = 16.f;
    float wideBtnW = btnW * 2 + gap, tallBtnH = btnH * 2 + gap;

    auto createMenuButton = [&](const sf::Texture& icon, sf::Vector2f iconScale, UI::Button::IconPlacement iconAlign,
        const std::string& label, unsigned int textSize, sf::Vector2f size, std::function<void()> onClick) {
        auto btn = std::make_unique<UI::Button>(icon, font, label, size, iconAlign);
        btn->setBackgroundTextures(&ResourceManager::get("button"), &ResourceManager::get("button-hover"), &ResourceManager::get("button-active"), nullptr, 32.0f);
        btn->setIconScale(iconScale);
        btn->setTextSize(textSize);
        btn->setCallback(std::move(onClick));
        return btn;
    };

    btnsCont->addChild(createMenuButton(ResourceManager::get("icon-play"), { 1.f, 1.f }, UI::Button::IconPlacement::Right, "ИГРАТЬ", 48, sf::Vector2f(wideBtnW, tallBtnH), [this]() {
        state = MenuState::LevelSelect;
    }));

    auto bottomGrid = std::make_unique<UI::Container>(sf::Vector2f(wideBtnW, tallBtnH));
    bottomGrid->setDirection(UI::Container::Direction::Row);
    bottomGrid->setContentAlign(UI::Container::ContentAlign::Center);
    bottomGrid->setItemAlign(UI::Container::ItemAlign::Center);
    bottomGrid->setGap(gap);

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

    bottomGrid->addChild(createMenuButton(ResourceManager::get("icon-upgrades"), { 0.75f, 0.75f }, UI::Button::IconPlacement::Top, "УЛУЧШЕНИЯ", 24, sf::Vector2f(btnW, tallBtnH), [this]() {
        state = MenuState::Upgrades;
    }));

    btnsCont->addChild(std::move(bottomGrid));
    root->addChild(std::move(btnsCont));

    return root;
}

std::unique_ptr<UI::Container> Menu::createLevelSelectMenu() {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());

    UI::Container* levelContent = nullptr;
    UI::Container* levelNav = nullptr;
    auto root = createSubMenu("ВЫБОР УРОВНЯ", &levelContent, &levelNav);

    if (levelContent) {
        levelContent->setDirection(UI::Container::Direction::Column);
        levelContent->setGap(10.f);

        // Верхний подконтейнер со звездами (аналог валюты в улучшениях)
        auto starsInfo = std::make_unique<UI::Container>(sf::Vector2f(250.f, 64.f));
        starsInfo->setDirection(UI::Container::Direction::Row);
        starsInfo->setContentAlign(UI::Container::ContentAlign::Center);
        starsInfo->setItemAlign(UI::Container::ItemAlign::Center);
        starsInfo->setGap(15.f);
        starsInfo->setBackgroundTexture(ResourceManager::get("panel-lighter"), 32.f);

        starsInfo->addChild(std::make_unique<UI::Image>(ResourceManager::get("icon-star-filled"), sf::Vector2f(48.f, 48.f)));

        int earnedStars = 0;
        for (const auto& lvl : levels) earnedStars += saveManager.getStars(lvl.id);
        int totalStars = (int)levels.size() * 3;

        auto starsText = std::make_unique<UI::Text>(font, std::to_string(earnedStars) + " / " + std::to_string(totalStars), 24);
        totalStarsTextPtr = starsText.get(); // Сохраняем указатель для обновления
        starsInfo->addChild(std::move(starsText));

        levelContent->addChild(std::move(starsInfo));

        // Нижний прокручиваемый подконтейнер с карточками уровней
        auto scrollArea = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 350.f));
        scrollArea->setDirection(UI::Container::Direction::Row);
        scrollArea->setWrap(true); // Разрешаем перенос карточек на новую строку
        scrollArea->setContentAlign(UI::Container::ContentAlign::Center);
        scrollArea->setItemAlign(UI::Container::ItemAlign::Center);
        scrollArea->setGap(25.f);
        scrollArea->setPadding({ 10.f, 5.f });
        scrollArea->setScrollEnabled(true);
        cardsArea = scrollArea.get(); // Ссылка для управления выделением

        levelCardMap.clear();

        for (const auto& level : levels) {
            sf::Vector2f cardSize(325.f, 225.f);
            auto card = std::make_unique<UI::Container>(cardSize);
            LevelCardWidgets widgets;
            widgets.root = card.get();

            card->setDirection(UI::Container::Direction::Column);
            card->setItemAlign(UI::Container::ItemAlign::Center);
            card->setPadding({ 10.f, 10.f });
            card->setGap(5.f);

            bool unlocked = saveManager.isUnlocked(level.id);
            widgets.wasUnlocked = unlocked;
            card->setBackgroundTexture(ResourceManager::get("card"), 16.f);
            if (!unlocked) card->setBackgroundColor(sf::Color(100, 100, 100, 150));

            auto numText = std::make_unique<UI::Text>(font, "УРОВЕНЬ " + std::to_string(level.index + 1), 24);
            numText->setColor(unlocked ? Colors::Theme::TextDark : sf::Color(50, 50, 50));
            card->addChild(std::move(numText));

            auto nameText = std::make_unique<UI::Text>(font, level.name, 26);
            nameText->setColor(unlocked ? Colors::Theme::TextMain : sf::Color(80, 80, 80));
            card->addChild(std::move(nameText));

            if (unlocked) {
                int maxWave = saveManager.getMaxWave(level.id);
                int bestScore = saveManager.getBestScore(level.id);

                auto recHeader = std::make_unique<UI::Text>(font, "РЕКОРД", 20);
                recHeader->setColor(Colors::Theme::TextYellow);
                card->addChild(std::move(recHeader));

                auto waveRec = std::make_unique<UI::Text>(font, "Волна: " + std::to_string(maxWave), 18);
                waveRec->setColor(sf::Color(200, 200, 200));
                widgets.waveText = waveRec.get();
                card->addChild(std::move(waveRec));

                auto scoreRec = std::make_unique<UI::Text>(font, "Счет: " + std::to_string(bestScore), 18);
                scoreRec->setColor(sf::Color(200, 200, 200));
                widgets.scoreText = scoreRec.get();
                card->addChild(std::move(scoreRec));

                auto starsRow = std::make_unique<UI::Container>(sf::Vector2f(cardSize.x, 30.f));
                starsRow->setDirection(UI::Container::Direction::Row);
                starsRow->setContentAlign(UI::Container::ContentAlign::Center);
                starsRow->setItemAlign(UI::Container::ItemAlign::Center);
                starsRow->setGap(10.f);

                int savedStars = saveManager.getStars(level.id);
                for (int i = 0; i < 3; ++i) {
                    const auto& tex = (i < savedStars) ? ResourceManager::get("icon-star-filled") : ResourceManager::get("icon-star-empty");
                    auto starImg = std::make_unique<UI::Image>(tex, sf::Vector2f(48.f, 48.f));
                    if (i >= savedStars) starImg->setColor(sf::Color(100, 100, 100, 100));
                    starsRow->addChild(std::move(starImg));
                }
                widgets.starsRow = starsRow.get();
                card->addChild(std::move(starsRow));
            } else {
                auto lockText = std::make_unique<UI::Text>(font, "ЗАБЛОКИРОВАНО", 18);
                lockText->setColor(sf::Color::Red);
                widgets.lockText = lockText.get();
                card->addChild(std::move(lockText));
            }

            auto clicker = std::make_unique<UI::Button>(font, "", cardSize);
            clicker->setTransparent(true);
            clicker->setFollowsLayout(false);
            clicker->setEnabled(unlocked);
            clicker->setCallback([this, path = level.filePath, area = cardsArea]() {
                if (area && !area->isCurrentlyDragging()) { selectedLevel = path; updateCardsSelection(); }
                });
            widgets.clicker = clicker.get();
            card->addChild(std::move(clicker));

            levelCardMap[level.id] = widgets;
            scrollArea->addChild(std::move(card));
        }
        levelContent->addChild(std::move(scrollArea));
    }

    if (levelNav) {
        auto startGameBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-play"), font, "ИГРАТЬ", sf::Vector2f(220.f, 60.f), UI::Button::IconPlacement::Right);
        startGameBtn->setBackgroundTextures(&ResourceManager::get("button"), &ResourceManager::get("button-hover"), &ResourceManager::get("button-active"), &ResourceManager::get("button-disabled"), 32.0f);
        startGameBtn->setIconScale({ 0.5f, 0.5f });
        startGameBtn->setCallback([this]() { if (!selectedLevel.empty()) levelChosen = true; });
        startGameBtn->setEnabled(false);
        playBtnPtr = startGameBtn.get();
        levelNav->addChild(std::move(startGameBtn));
    }

    return root;
}

std::unique_ptr<UI::Container> Menu::createSettingsMenu() {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());

    UI::Container* settingsContent = nullptr;
    UI::Container* settingsNav = nullptr;
    auto root = createSubMenu("НАСТРОЙКИ", &settingsContent, &settingsNav);

    if (settingsContent) {
        settingsContent->setDirection(UI::Container::Direction::Column);
        settingsContent->setContentAlign(UI::Container::ContentAlign::Center);
        settingsContent->setItemAlign(UI::Container::ItemAlign::Center);
        settingsContent->setBackgroundTexture(ResourceManager::get("panel-light"), 32.f);
        settingsContent->setGap(25.f);
        settingsContent->setPadding({ 10.f, 5.f });
        settingsContent->setScrollEnabled(true);

        auto createHeader = [&](const std::string& text) {
            auto header = std::make_unique<UI::Text>(font, text, 32);
            header->setColor(sf::Color::White);
            header->setAlignment(UI::Text::Align::Left);
            auto cont = std::make_unique<UI::Container>(sf::Vector2f(900.f, 48.f));
            cont->setDirection(UI::Container::Direction::Row);
            cont->setContentAlign(UI::Container::ContentAlign::Center);
            cont->setItemAlign(UI::Container::ItemAlign::Center);
            cont->addChild(std::move(header));
            return cont;
        };

        auto createRow = [&](const sf::Texture& iconTex, const std::string& label, std::unique_ptr<UI::Widget> control) {
            auto row = std::make_unique<UI::Container>(sf::Vector2f(950.f, 64.f));
            row->setDirection(UI::Container::Direction::Row);
            row->setContentAlign(UI::Container::ContentAlign::Center);
            row->setItemAlign(UI::Container::ItemAlign::Center);
            row->setGap(10.f);
            row->addChild(std::make_unique<UI::Image>(iconTex, sf::Vector2f(48.f, 48.f)));
            auto text = std::make_unique<UI::Text>(font, label, 24, sf::Vector2f(350.f, 64.f));
            text->setColor(Colors::Theme::TextMain);
            row->addChild(std::move(text));
            row->addChild(std::move(control));
            return row;
        };

        auto createToggleRow = [&](const sf::Texture& iconTex, const std::string& label, bool& tmpVar) {
            auto row = std::make_unique<UI::Container>(sf::Vector2f(950.f, 64.f));
            row->setDirection(UI::Container::Direction::Row);
            row->setContentAlign(UI::Container::ContentAlign::Center);
            row->setItemAlign(UI::Container::ItemAlign::Center);
            row->setGap(10.f);
            row->addChild(std::make_unique<UI::Image>(iconTex, sf::Vector2f(48.f, 48.f)));

            auto text = std::make_unique<UI::Text>(font, label, 24, sf::Vector2f(350.f, 64.f));
            text->setColor(Colors::Theme::TextMain);
            row->addChild(std::move(text));

            auto toggleCont = std::make_unique<UI::Container>(sf::Vector2f(350.f, 32.f));
            toggleCont->setDirection(UI::Container::Direction::Row);
            toggleCont->setContentAlign(UI::Container::ContentAlign::Center);
            toggleCont->setItemAlign(UI::Container::ItemAlign::Center);
            auto toggleBtn = std::make_unique<UI::Toggle>(tmpVar, sf::Vector2f(80.f, 32.f));
            toggleBtn->setCallback([&tmpVar](bool state) {
                tmpVar = state;
            });
            toggleCont->addChild(std::move(toggleBtn));
            row->addChild(std::move(toggleCont));
            return row;
        };

        // АУДИО
        auto audioCard = std::make_unique<UI::Container>(sf::Vector2f(950.f, 212.f));
        audioCard->setDirection(UI::Container::Direction::Column);
        audioCard->setContentAlign(UI::Container::ContentAlign::Center);
        audioCard->setItemAlign(UI::Container::ItemAlign::Center);
        audioCard->setPadding({10.f, 10.f});
        audioCard->setGap(10.f);
        audioCard->addChild(createHeader("АУДИО"));

        auto musicSlider = std::make_unique<UI::Slider>(font, 0.f, 100.f, (float)tmpMusicVol, sf::Vector2f(350.f, 32.f));
        musicSliderPtr = musicSlider.get();
        musicSlider->setCallback([this](float value) { tmpMusicVol = (int)value; });
        audioCard->addChild(createRow(ResourceManager::get("icon-music"), "ГРОМКОСТЬ МУЗЫКИ", std::move(musicSlider)));

        auto sfxSlider = std::make_unique<UI::Slider>(font, 0.f, 100.f, (float)tmpSfxVol, sf::Vector2f(350.f, 32.f));
        sfxSliderPtr = sfxSlider.get();
        sfxSlider->setCallback([this](float value) { tmpSfxVol = (int)value; });
        audioCard->addChild(createRow(ResourceManager::get("icon-audio"), "ГРОМКОСТЬ ЗВУКОВ", std::move(sfxSlider)));
        settingsContent->addChild(std::move(audioCard));

        // ИНТЕРФЕЙС
        auto uiCard = std::make_unique<UI::Container>(sf::Vector2f(950.f, 212.f));
        uiCard->setDirection(UI::Container::Direction::Column);
        uiCard->setContentAlign(UI::Container::ContentAlign::Center);
        uiCard->setItemAlign(UI::Container::ItemAlign::Center);
        uiCard->setPadding({10.f, 10.f});
        uiCard->setGap(10.f);
        uiCard->addChild(createHeader("ИНТЕРФЕЙС"));

        auto sensSlider = std::make_unique<UI::Slider>(font, 0.5f, 3.0f, tmpSensitivity, sf::Vector2f(350.f, 32.f));
        sensSlider->setPrecision(1);
        sensSliderPtr = sensSlider.get();
        sensSlider->setCallback([this](float value) { tmpSensitivity = value; });
        uiCard->addChild(createRow(ResourceManager::get("icon-sensivity"), "ЧУВСТВИТЕЛЬНОСТЬ", std::move(sensSlider)));

        auto uiScaleSlider = std::make_unique<UI::Slider>(font, 0.6f, 1.6f, tmpUiScale, sf::Vector2f(350.f, 32.f));
        uiScaleSlider->setPrecision(1);
        uiScaleSliderPtr = uiScaleSlider.get();
        uiScaleSlider->setCallback([this](float value) { tmpUiScale = value; });
        uiCard->addChild(createRow(ResourceManager::get("icon-display"), "МАСШТАБ ИНТЕРФЕЙСА", std::move(uiScaleSlider)));
        settingsContent->addChild(std::move(uiCard));

        // ГРАФИКА
#ifndef __ANDROID__
        auto graphicsCard = std::make_unique<UI::Container>(sf::Vector2f(950.f, 212.f));
        graphicsCard->setDirection(UI::Container::Direction::Column);
        graphicsCard->setContentAlign(UI::Container::ContentAlign::Center);
        graphicsCard->setItemAlign(UI::Container::ItemAlign::Center);
        graphicsCard->setPadding({10.f, 10.f});
        graphicsCard->setGap(10.f);
        graphicsCard->addChild(createHeader("ГРАФИКА"));

        graphicsCard->addChild(createToggleRow(ResourceManager::get("icon-vsync"), "ВЕРТИКАЛЬНАЯ СИНХР.", tmpVsync));
        graphicsCard->addChild(createToggleRow(ResourceManager::get("icon-fullscreen"), "ПОЛНОЭКРАННЫЙ РЕЖИМ", tmpFullscreen));
        settingsContent->addChild(std::move(graphicsCard));
#endif

        // ВИЗУАЛЬНЫЕ ЭФФЕКТЫ
        auto vfxCard = std::make_unique<UI::Container>(sf::Vector2f(950.f, 360.f));
        vfxCard->setDirection(UI::Container::Direction::Column);
        vfxCard->setContentAlign(UI::Container::ContentAlign::Center);
        vfxCard->setItemAlign(UI::Container::ItemAlign::Center);
        vfxCard->setPadding({10.f, 10.f});
        vfxCard->setGap(10.f);
        vfxCard->addChild(createHeader("ВИЗУАЛЬНЫЕ ЭФФЕКТЫ"));

        vfxCard->addChild(createToggleRow(ResourceManager::get("icon-hit"), "ЭФФЕКТЫ ПОПАДАНИЯ", tmpVfxHit));
        vfxCard->addChild(createToggleRow(ResourceManager::get("icon-trail"), "ТРАССЕРЫ СНАРЯДОВ", tmpVfxTrail));
        vfxCard->addChild(createToggleRow(ResourceManager::get("icon-explosion"), "ВЗРЫВЫ ПРИ СМЕРТИ", tmpVfxDeath));
        vfxCard->addChild(createToggleRow(ResourceManager::get("icon-animation"), "АНИМАЦИИ", tmpAnimation));
        settingsContent->addChild(std::move(vfxCard));
    }

    if (settingsNav) {
        auto saveBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-save"), font, "СОХРАНИТЬ", sf::Vector2f(220.f, 64.f), UI::Button::IconPlacement::Right);
        saveBtn->setBackgroundTextures(&ResourceManager::get("button"), &ResourceManager::get("button-hover"), &ResourceManager::get("button-active"), nullptr, 32.0f);
        saveBtn->setIconScale({ 0.5f, 0.5f });
        saveBtn->setCallback([this]() {
            bool oldFullscreen = settings.get<bool>("fullscreen", false);
            bool fullscreenChanged = (oldFullscreen != tmpFullscreen);

            settings.set<int>("music_volume", tmpMusicVol); settings.set<int>("sfx_volume", tmpSfxVol);
            settings.set<float>("sensitivity", tmpSensitivity); settings.set<float>("ui_scale", tmpUiScale);
            settings.set<bool>("fullscreen", tmpFullscreen);
            settings.set<bool>("vsync", tmpVsync);
            settings.set<bool>("vfx_hit", tmpVfxHit);
            settings.set<bool>("vfx_trail", tmpVfxTrail);
            settings.set<bool>("vfx_death", tmpVfxDeath);
            settings.set<bool>("animation", tmpAnimation);
            settings.save();
            if (fullscreenChanged) {
                windowRecreationRequired = true;
                Engine::Logger::info("Запрошено пересоздание окна (смена режима на {})", tmpFullscreen ? "Fullscreen" : "Windowed");
            }
            window.setVerticalSyncEnabled(tmpVsync);
            updateViewSizes(window.getSize());
            state = MenuState::Main;
            });
        settingsNav->addChild(std::move(saveBtn));
    }

    return root;
}



// Обновление всех карточек уровней на основе текущего сохранения
void Menu::refreshLevelCards() {
    for (const auto& level : levels) {
        if (levelCardMap.find(level.id) == levelCardMap.end()) continue;
        
        auto& w = levelCardMap[level.id];
        bool currentlyUnlocked = saveManager.isUnlocked(level.id);

        // Обработка момента разблокировки
        if (currentlyUnlocked && !w.wasUnlocked) {
            initUI(); 
            return;
        }

        // Обновление текстовых данных (рекордов)
        if (currentlyUnlocked) {
            if (w.waveText) w.waveText->setText("Волна: " + std::to_string(saveManager.getMaxWave(level.id)));
            if (w.scoreText) w.scoreText->setText("Счет: " + std::to_string(saveManager.getBestScore(level.id)));

            // 3. Обновление звезд
            if (w.starsRow) {
                int savedStars = saveManager.getStars(level.id);
                w.starsRow->clearChildren();
                for (int i = 0; i < 3; ++i) {
                    const auto& tex = (i < savedStars) ? ResourceManager::get("icon-star-filled") : ResourceManager::get("icon-star-empty");
                    auto starImg = std::make_unique<UI::Image>(tex, sf::Vector2f(64.f, 64.f));
                    if (i >= savedStars) starImg->setColor(sf::Color(100, 100, 100, 100));
                    w.starsRow->addChild(std::move(starImg));
                }
                w.starsRow->rebuild();
            }
        }
    }

    if (totalStarsTextPtr) {
        int earnedStars = 0;
        int totalStars = levels.size() * 3;
        for (const auto& level : levels) {
            earnedStars += saveManager.getStars(level.id);
        }
        totalStarsTextPtr->setText(std::to_string(earnedStars) + " / " + std::to_string(totalStars));
    }
}

std::unique_ptr<UI::Container> Menu::createSubMenu(const std::string& title, UI::Container** outContent, UI::Container** outNav) {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());
    auto root = std::make_unique<UI::Container>(winSize);
    root->setDirection(UI::Container::Direction::Column);
    root->setItemAlign(UI::Container::ItemAlign::Center);
    root->setContentAlign(UI::Container::ContentAlign::Center);
    root->setBackgroundTexture(ResourceManager::get("panel"), 32.f);
    root->setPadding({ 0.f, 5.f });
    root->setGap(20.f);

    auto header = std::make_unique<UI::Text>(font, title, 60, sf::Vector2f(winSize.x * 0.9f, 80.f));
    header->setAlignment(UI::Text::Align::Center);
    header->setColor(Colors::Theme::TextMain);
    root->addChild(std::move(header));

    auto content = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 400.f)); 
    content->setDirection(UI::Container::Direction::Column);
    content->setItemAlign(UI::Container::ItemAlign::Center);
    content->setContentAlign(UI::Container::ContentAlign::Center);
    content->setBackgroundTexture(ResourceManager::get("panel-light"), 32.f);
    if (outContent) *outContent = content.get();
    root->addChild(std::move(content));

    auto nav = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 80.f));
    nav->setDirection(UI::Container::Direction::Row);
    nav->setContentAlign(UI::Container::ContentAlign::Center);
    nav->setItemAlign(UI::Container::ItemAlign::Center);
    nav->setGap(30.f);
    auto back = std::make_unique<UI::Button>(ResourceManager::get("icon-back"), font, "НАЗАД", sf::Vector2f(220.f, 60.f), UI::Button::IconPlacement::Left);
    back->setBackgroundTextures(&ResourceManager::get("button"), &ResourceManager::get("button-hover"), &ResourceManager::get("button-active"), nullptr, 32.0f);
    back->setIconScale({ 0.5f, 0.5f });
    back->setCallback([this]() { state = MenuState::Main; selectedLevel = ""; updateCardsSelection(); });
    nav->addChild(std::move(back));
    if (outNav) *outNav = nav.get();
    root->addChild(std::move(nav));
    return root;
}

std::unique_ptr<UI::Container> Menu::createUpgradeMenu() {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());
    auto root = std::make_unique<UI::Container>(winSize);
    root->setDirection(UI::Container::Direction::Column);
    root->setItemAlign(UI::Container::ItemAlign::Center);
    root->setContentAlign(UI::Container::ContentAlign::Center);
    root->setBackgroundTexture(ResourceManager::get("panel"), 32.f);
    root->setPadding({ 10.f, 5.f });
    root->setGap(10.f);

    auto header = std::make_unique<UI::Text>(font, "УЛУЧШЕНИЯ", 60, sf::Vector2f(winSize.x * 0.9f, 80.f));
    header->setAlignment(UI::Text::Align::Center);
    header->setColor(Colors::Theme::TextMain);
    root->addChild(std::move(header));

    auto mainBox = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 400.f));
    mainBox->setDirection(UI::Container::Direction::Column);
    mainBox->setItemAlign(UI::Container::ItemAlign::Center);
    mainBox->setContentAlign(UI::Container::ContentAlign::Center);
    mainBox->setBackgroundTexture(ResourceManager::get("panel-light"), 32.f);
    mainBox->setGap(10.f);

    auto currency = std::make_unique<UI::Container>(sf::Vector2f(250.f, 64.f));
    currency->setDirection(UI::Container::Direction::Row);
    currency->setContentAlign(UI::Container::ContentAlign::Center);
    currency->setItemAlign(UI::Container::ItemAlign::Center);
    currency->setGap(15.f);
    currency->addChild(std::make_unique<UI::Image>(ResourceManager::get("icon-money"), sf::Vector2f(48.f, 48.f)));
    currency->setBackgroundTexture(ResourceManager::get("panel-lighter"), 32.f);

    auto moneyText = std::make_unique<UI::Text>(font, std::to_string(saveManager.getMoney()), 24);
    moneyTextPtr = moneyText.get();
    currency->addChild(std::move(moneyText));
    mainBox->addChild(std::move(currency));

    auto scrollArea = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 300.f));
    scrollArea->setDirection(UI::Container::Direction::Column);
    scrollArea->setItemAlign(UI::Container::ItemAlign::Center);
    scrollArea->setScrollEnabled(true);
    scrollArea->setGap(20.f);
    scrollArea->setPadding({ 10.f, 5.f });

    auto sec1H = std::make_unique<UI::Text>(font, "МОДЕРНИЗАЦИЯ ТУРЕЛЕЙ", 36);
    sec1H->setColor(Colors::Theme::TextYellow);
    scrollArea->addChild(std::move(sec1H));

    auto turretGrid = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 625.f));
    turretGrid->setDirection(UI::Container::Direction::Row);
    turretGrid->setWrap(true);
    turretGrid->setGap(20.f);
    turretGrid->setContentAlign(UI::Container::ContentAlign::Center);
    turretGrid->setItemAlign(UI::Container::ItemAlign::Center);

    std::vector<std::string> towerTypes = GameData::getTowerNames();
    std::vector<std::string> statNames = { "РАНГ", "АТАКА", "ПРОБИТИЕ", "СКОР. АТАКИ", "РАДИУС", "УРОВЕНЬ" };
    upgradeValuePtrs.clear(); upgradeValuePtrs.resize(towerTypes.size());
    upgradeCostPtrs.clear(); upgradeCostPtrs.resize(towerTypes.size());
    upgradeBtnPtrs.clear(); upgradeBtnPtrs.resize(towerTypes.size());

    for (int t = 0; t < (int)towerTypes.size(); ++t) {
        auto towerCard = std::make_unique<UI::Container>(sf::Vector2f(550.f, 320.f)); // Немного увеличил высоту под 6-й стат
        towerCard->setDirection(UI::Container::Direction::Row);
        towerCard->setContentAlign(UI::Container::ContentAlign::Center);
        towerCard->setItemAlign(UI::Container::ItemAlign::Center);
        towerCard->setGap(20.f);
        towerCard->setBackgroundTexture(ResourceManager::get("card"), 16.f);

        // Левая часть: название и картинка
        auto leftCol = std::make_unique<UI::Container>(sf::Vector2f(140.f, 300.f));
        leftCol->setDirection(UI::Container::Direction::Column);
        leftCol->setContentAlign(UI::Container::ContentAlign::Center);
        leftCol->setItemAlign(UI::Container::ItemAlign::Center);
        leftCol->setGap(15.f);

        std::string upperName = towerTypes[t];
        std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
        auto tName = std::make_unique<UI::Text>(font, upperName, 24);
        tName->setColor(Colors::Theme::TextMain);
        leftCol->addChild(std::move(tName));

        auto tImg = std::make_unique<UI::Image>(ResourceManager::get("tower-" + towerTypes[t] + "-preview"), sf::Vector2f(96.f, 96.f));
        leftCol->addChild(std::move(tImg));

        towerCard->addChild(std::move(leftCol));

        // Правая часть: статы
        auto rightCol = std::make_unique<UI::Container>(sf::Vector2f(360.f, 300.f));
        rightCol->setDirection(UI::Container::Direction::Column);
        rightCol->setContentAlign(UI::Container::ContentAlign::Center);
        rightCol->setItemAlign(UI::Container::ItemAlign::Center);
        rightCol->setGap(2.f);

        for (size_t s = 0; s < statNames.size(); ++s) {
            auto statRow = std::make_unique<UI::Container>(sf::Vector2f(360.f, 44.f)); // Немного уменьшил высоту строки под 6 статов
            statRow->setDirection(UI::Container::Direction::Row);
            statRow->setContentAlign(UI::Container::ContentAlign::Center);
            statRow->setItemAlign(UI::Container::ItemAlign::Center);
            
            auto sName = std::make_unique<UI::Text>(font, statNames[s], 18, sf::Vector2f(150.f, 32.f));
            sName->setAlignment(UI::Text::Align::Left);
            statRow->addChild(std::move(sName));

            auto sVal = std::make_unique<UI::Text>(font, "0", 18, sf::Vector2f(60.f, 32.f));
            sVal->setAlignment(UI::Text::Align::Left);
            upgradeValuePtrs[t].push_back(sVal.get());
            statRow->addChild(std::move(sVal));

            auto sCost = std::make_unique<UI::Text>(font, "0", 18, sf::Vector2f(70.f, 32.f));
            sCost->setAlignment(UI::Text::Align::Left);
            upgradeCostPtrs[t].push_back(sCost.get());
            statRow->addChild(std::move(sCost));

            auto uBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-upgrade2"), sf::Vector2f(50.f, 36.f));
            uBtn->setBackgroundTextures(&ResourceManager::get("button-flat"), &ResourceManager::get("button-flat-hover"), &ResourceManager::get("button-flat"), &ResourceManager::get("button-flat-disabled"), 16.0f);
            uBtn->setIconScale({ 0.3f , 0.3f });
            auto tType = towerTypes[t];
            UpgradeManager* um = &upgradeManager;
            SaveManager* sm = &saveManager;
            uBtn->setCallback([um, sm, tType, sIndex = s]() {
                std::string keys[] = {"rank", "damage", "pierce", "firerate", "range", "level"};
                if (um->isStatAtLimit(tType, keys[sIndex])) return;
                int cost = um->getUpgradeCost(tType, (int)sIndex);
                if (sm->spendMoney(cost)) {
                    if (sIndex == 0) um->upgradeRank(tType);
                    else if (sIndex == 1) um->upgradeDamage(tType, 0.025f);
                    else if (sIndex == 2) um->upgradePierce(tType, 0.025f);
                    else if (sIndex == 3) um->upgradeFirerate(tType, 0.025f);
                    else if (sIndex == 4) um->upgradeRange(tType, 0.025f);
                    else if (sIndex == 5) um->upgradeMaxLevel(tType);
                }
            });
            upgradeBtnPtrs[t].push_back(uBtn.get());
            statRow->addChild(std::move(uBtn));
            rightCol->addChild(std::move(statRow));
        }
        
        towerCard->addChild(std::move(rightCol));
        turretGrid->addChild(std::move(towerCard));
    }
    scrollArea->addChild(std::move(turretGrid));

    auto sec2H = std::make_unique<UI::Text>(font, "СТРАТЕГИЧЕСКИЙ ОТДЕЛ", 36);
    sec2H->setColor(Colors::Theme::TextYellow);
    scrollArea->addChild(std::move(sec2H));

    auto stratCont = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 250.f));
    stratCont->setDirection(UI::Container::Direction::Column);
    stratCont->setItemAlign(UI::Container::ItemAlign::Center);
    stratCont->setItemAlign(UI::Container::ItemAlign::Center);
    stratCont->setGap(10.f);

    struct MetaUp { std::string name, icon; int costStep; };
    std::vector<MetaUp> metaUps = { { "НАЧАЛЬНЫЙ КАПИТАЛ", "icon-coins", 500 }, { "ДОХОДНОСТЬ", "icon-money", 750 }, { "ЖИЗНИ БАЗЫ", "icon-heart", 400 } };
    metaValuePtrs.clear(); metaCostPtrs.clear(); metaBtnPtrs.clear();

    for (int i = 0; i < 3; ++i) {
        auto row = std::make_unique<UI::Container>(sf::Vector2f(550.f, 48.f));
        row->setDirection(UI::Container::Direction::Row);
        row->setContentAlign(UI::Container::ContentAlign::Center);
        row->setItemAlign(UI::Container::ItemAlign::Center);
        row->setGap(10.f);
        row->setBackgroundTexture(ResourceManager::get("card"), 8.f);
        row->addChild(std::make_unique<UI::Image>(ResourceManager::get(metaUps[i].icon), sf::Vector2f(40.f, 40.f)));
        auto nLabel = std::make_unique<UI::Text>(font, metaUps[i].name, 20, sf::Vector2f(250.f, 40.f));
        nLabel->setAlignment(UI::Text::Align::Left);
        row->addChild(std::move(nLabel));
        auto vText = std::make_unique<UI::Text>(font, "Ур. 0", 20, sf::Vector2f(75.f, 40.f));
        metaValuePtrs.push_back(vText.get());
        row->addChild(std::move(vText));
        auto cText = std::make_unique<UI::Text>(font, "500", 20, sf::Vector2f(75.f, 40.f));
        cText->setColor(Colors::Theme::TextYellow);
        metaCostPtrs.push_back(cText.get());
        row->addChild(std::move(cText));
        auto uBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-upgrade2"), sf::Vector2f(50.f, 36.f));
        uBtn->setBackgroundTextures(&ResourceManager::get("button-flat"), &ResourceManager::get("button-flat-hover"), &ResourceManager::get("button-flat"), &ResourceManager::get("button-flat-disabled"), 16.0f);
        uBtn->setIconScale({ 0.3f, 0.3f });
        SaveManager* sm = &saveManager;
        UpgradeManager* um = &upgradeManager;
        std::string metaId = (i == 0 ? "globalCoins" : (i == 1 ? "globalMoney" : "globalBaseHp"));
        uBtn->setCallback([um, sm, metaId]() {
            int cost = um->getMetaUpgradeCost(metaId);
            if (sm->spendMoney(cost)) {
                um->upgradeMeta(metaId);
            }
        });
        metaBtnPtrs.push_back(uBtn.get());
        row->addChild(std::move(uBtn));
        stratCont->addChild(std::move(row));
    }
    scrollArea->addChild(std::move(stratCont));
    mainBox->addChild(std::move(scrollArea));
    root->addChild(std::move(mainBox));

    auto nav = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 80.f));
    nav->setDirection(UI::Container::Direction::Row);
    nav->setContentAlign(UI::Container::ContentAlign::Center);
    nav->setItemAlign(UI::Container::ItemAlign::Center);
    auto back = std::make_unique<UI::Button>(ResourceManager::get("icon-back"), font, "НАЗАД", sf::Vector2f(220.f, 60.f), UI::Button::IconPlacement::Left);
    back->setBackgroundTextures(&ResourceManager::get("button"), &ResourceManager::get("button-hover"), &ResourceManager::get("button-active"), nullptr, 32.0f);
    back->setIconScale({ 0.5f, 0.5f });
    back->setCallback([this]() { state = MenuState::Main; });
    nav->addChild(std::move(back));
    root->addChild(std::move(nav));
    return root;
}

void Menu::updateCardsSelection() {
    if (!cardsArea) return;
    for (size_t i = 0; i < cardsArea->getChildrenCount(); ++i) {
        auto* card = static_cast<UI::Container*>(cardsArea->getChild(i));
        if (i >= levels.size()) continue;
        card->setBackgroundTexture(ResourceManager::get(levels[i].filePath == selectedLevel ? "card-light" : "card"), 16.f);
    }
    if (playBtnPtr) playBtnPtr->setEnabled(!selectedLevel.empty());
}

void Menu::handleEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) window.close();
        if (const auto* resized = event->getIf<sf::Event::Resized>()) updateViewSizes(resized->size);
        UI::Container* current = nullptr; 
        if (state == MenuState::Main) current = mainContainer.get();
        else if (state == MenuState::LevelSelect) current = levelContainer.get();
        else if (state == MenuState::Settings) current = settingsContainer.get();
        else if (state == MenuState::Upgrades) current = upgradesContainer.get();
        if (current) current->handleEvent(*event, window, uiView);
    }
}

void Menu::render() {
    window.clear(Colors::Theme::Background);
    window.setView(uiView);
    UI::Container* current = nullptr;
    if (state == MenuState::Main) current = mainContainer.get();
    else if (state == MenuState::LevelSelect) current = levelContainer.get();
    else if (state == MenuState::Settings) current = settingsContainer.get();
    else if (state == MenuState::Upgrades) current = upgradesContainer.get();
    if (current) current->render(window);
    
    if (state == MenuState::Upgrades && moneyTextPtr) {
        moneyTextPtr->setText(std::to_string(saveManager.getMoney()));
        std::vector<std::string> tTypes = GameData::getTowerNames();
        for (size_t t = 0; t < upgradeValuePtrs.size() && t < tTypes.size(); ++t) {
            const auto* towerUp = upgradeManager.getUpgrade(tTypes[t]);
            if (!towerUp) continue;

            const UpgradeManager::Upgrade* stats[] = { &towerUp->rank, &towerUp->damage, &towerUp->pierce, &towerUp->firerate, &towerUp->range, &towerUp->level };
            std::string keys[] = { "rank", "damage", "pierce", "firerate", "range", "level" };

            for (size_t s = 0; s < 6; ++s) {
                if (s >= upgradeValuePtrs[t].size()) continue;

                std::string vStr;
                if (s == 0 || s == 5) {
                    // Ранг и Уровень - просто число
                    vStr = std::to_string(stats[s]->level);
                } else {
                    // Статы - множитель или абсолютное значение, если нужно. 
                    // Пока оставляем как было (отображаем value)
                    vStr = std::to_string(stats[s]->value);
                    if (vStr.find('.') != std::string::npos) vStr = vStr.substr(0, vStr.find('.') + 3);
                }
                
                upgradeValuePtrs[t][s]->setText(vStr);
                int cost = upgradeManager.getUpgradeCost(tTypes[t], (int)s);
                bool atLimit = upgradeManager.isStatAtLimit(tTypes[t], keys[s]);
                
                upgradeCostPtrs[t][s]->setText(atLimit ? "MAX" : std::to_string(cost));
                upgradeCostPtrs[t][s]->setColor(atLimit ? sf::Color(150,150,150) : (saveManager.getMoney() >= cost ? Colors::Theme::TextGreen : Colors::Theme::TextRed));
                upgradeBtnPtrs[t][s]->setEnabled(!atLimit);
            }
        }

        const char* metaIds[] = { "globalCoins", "globalMoney", "globalBaseHp" };
        for (int i = 0; i < 3; ++i) {
            const auto* mUp = upgradeManager.getMetaUpgrade(metaIds[i]);
            if (!mUp || i >= (int)metaValuePtrs.size()) continue;

            metaValuePtrs[i]->setText("Ур. " + std::to_string(mUp->upgrade.level));
            int cost = upgradeManager.getMetaUpgradeCost(metaIds[i]);
            bool atLimit = mUp->upgrade.level >= mUp->upgrade.maxLevel;

            metaCostPtrs[i]->setText(atLimit ? "MAX" : std::to_string(cost));
            metaCostPtrs[i]->setColor(atLimit ? Colors::Theme::TextDark : (saveManager.getMoney() >= cost ? Colors::Theme::TextGreen : Colors::Theme::TextRed));
            metaBtnPtrs[i]->setEnabled(!atLimit);
        }
    }
    if (lastResult != SessionResult::None) resultOverlay->render(window);
    window.display();
}

void Menu::updateViewSizes(sf::Vector2u windowSize) {
    float sw = static_cast<float>(windowSize.x), sh = static_cast<float>(windowSize.y);
    float baseScale = sh / 1080.f;
    uiScale = baseScale * tmpUiScale;
    if (uiScale <= 0.1f) uiScale = 1.0f;
    float uiH = sh / uiScale, uiW = uiH * (sw / sh);
    uiView = sf::View(sf::FloatRect({ 0.f, 0.f }, { uiW, uiH }));
    sf::Vector2f rootSize(uiW * 0.9f, uiH * 0.95f), rootPos((uiW - rootSize.x) / 2.f, (uiH - rootSize.y) / 2.f);

    if (mainContainer) {
        float headerWidth = rootSize.x * 0.9f;
        if (headerContPtr) headerContPtr->setSize(sf::Vector2f(headerWidth, 125.f));
        
        if (titleTextPtr) {
            titleTextPtr->setSize(sf::Vector2f(headerWidth, titleTextPtr->getSize().y));
            // Подгоняем размер шрифта, чтобы название гарантированно влезало
            unsigned int targetSize = 80;
            titleTextPtr->setFontSize(targetSize);
            while (titleTextPtr->getLocalBounds().size.x > headerWidth * 0.9f && targetSize > 30) {
                targetSize -= 2;
                titleTextPtr->setFontSize(targetSize);
            }
        }

        if (btnsContPtr) btnsContPtr->setSize(sf::Vector2f(rootSize.x * 0.6f, 320.f));
        mainContainer->setSize(rootSize); mainContainer->setPosition(rootPos); mainContainer->rebuild(); 
    }

    auto updateSub = [&](std::unique_ptr<UI::Container>& cont) {
        if (cont) {
            float headerH = 80.f, navH = 80.f, gap = 20.f, p = 5.f;
            float contentH = rootSize.y - headerH - navH - gap * 2.f - p * 2.f;
            for (size_t i = 0; i < cont->getChildrenCount(); ++i) {
                auto* child = cont->getChild(i); if (!child) continue;
                float targetWidth = rootSize.x * 0.95f;
                if ((cont == upgradesContainer || cont == levelContainer || cont == settingsContainer) && i == 1) {
                    child->setSize(sf::Vector2f(targetWidth, contentH));
                    if (auto* asContainer = dynamic_cast<UI::Container*>(child)) {
                        if (asContainer->getChildrenCount() >= 2 && (cont == upgradesContainer || cont == levelContainer)) {
                            auto *cur = asContainer->getChild(0), *cards = asContainer->getChild(1);
                            if (cur) cur->setSize(sf::Vector2f(targetWidth, cur->getSize().y));
                            if (cards) {
                                cards->setSize(sf::Vector2f(targetWidth, contentH - (cur ? cur->getSize().y : 0.f) - 10.f));
                                // В upgradesContainer нужно также обновить ширину внутренних сеток, чтобы они делали wrap
                                if (cont == upgradesContainer) {
                                    if (auto* cardsContainer = dynamic_cast<UI::Container*>(cards)) {
                                        for (size_t j = 0; j < cardsContainer->getChildrenCount(); ++j) {
                                            auto* innerChild = cardsContainer->getChild(j);
                                            // Растягиваем только сетки (Container), текстовые заголовки (Text) не трогаем, чтобы не сбивать выравнивание
                                            if (dynamic_cast<UI::Container*>(innerChild)) {
                                                innerChild->setSize(sf::Vector2f(targetWidth, innerChild->getSize().y));
                                            }
                                        }
                                        // Обязательно перестраиваем сам скролл-контейнер, так как его дочерние элементы могли изменить свою высоту из-за переноса строк (wrap)
                                        cardsContainer->rebuild();
                                    }
                                }
                            }
                        }
                    }
                }
                else { child->setSize(sf::Vector2f(targetWidth, child->getSize().y)); }
            }
            cont->setSize(rootSize); cont->setPosition(rootPos); cont->rebuild();
        }
    };
    updateSub(levelContainer); updateSub(settingsContainer); updateSub(upgradesContainer);
    if (resultOverlay) { resultOverlay->setSize(rootSize); resultOverlay->setPosition(rootPos); resultOverlay->rebuild(); }
}

void Menu::scanLevels() {
    levels.clear();
#ifdef __ANDROID__
    // Логика для Android: используем AssetManager
    ANativeActivity* activity = sf::getNativeActivity();
    AAssetDir* assetDir = AAssetManager_openDir(activity->assetManager, "levels");
    if (!assetDir) {
        Logger::error("[Menu]: Не удалось открыть папку levels в assets");
        return;
    }

    const char* fileName = nullptr;
    while ((fileName = AAssetDir_getNextFileName(assetDir)) != nullptr) {
        std::string sName = fileName;
        if (sName.size() > 4 && sName.substr(sName.size() - 4) == ".map") {
            LevelInfo info;
            info.filePath = "levels/" + sName; // Путь внутри assets
            info.id = sName.substr(0, sName.find_last_of('.'));
            info.name = readLevelName(info.filePath);
            levels.push_back(info);
        }
    }
    AAssetDir_close(assetDir);

    // Сортируем уровни по ID (имени файла)
    std::sort(levels.begin(), levels.end(), [](const LevelInfo& a, const LevelInfo& b) {
        return a.id < b.id;
        });

    for (int i = 0; i < (int)levels.size(); i++) levels[i].index = i;

#else
    // Логика для Desktop: используем std::filesystem
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
}

std::string Menu::readLevelName(const std::string& path) const {
    auto content = readFile(path); if (!content || content->empty()) return "Безымянный";
    size_t pos = content->find("name="); if (pos == std::string::npos) return "Безымянный";
    size_t start = pos + 5, end = content->find("\n", start);
    std::string name = (end == std::string::npos) ? content->substr(start) : content->substr(start, end - start);
    if (!name.empty() && name.back() == '\r') name.pop_back();
    return name.empty() ? "Безымянный" : name;
}

void Menu::cleanup() {
    mainContainer.reset(); levelContainer.reset(); settingsContainer.reset(); upgradesContainer.reset(); resultOverlay.reset();
    upgradeValuePtrs.clear(); upgradeCostPtrs.clear(); upgradeBtnPtrs.clear(); moneyTextPtr = nullptr;
}

bool Menu::isLevelChosen() const { return levelChosen; }
std::string Menu::getChosenLevel() const { return selectedLevel; }
void Menu::resetChoice() { levelChosen = false; selectedLevel = ""; updateCardsSelection(); }
void Menu::resetLastResult() { lastResult = SessionResult::None; }
bool Menu::consumesWindowRecreationRequest() { bool req = windowRecreationRequired; windowRecreationRequired = false; return req; }
void Menu::notifyResult(SessionResult result, const std::string& levelPath) { lastResult = result; lastLevelPath = levelPath; }
int Menu::getMoney() const { return saveManager.getMoney(); }
UpgradeManager& Menu::getUpgradeManager() { return upgradeManager; }
