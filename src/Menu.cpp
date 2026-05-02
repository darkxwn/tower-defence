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
        upgradeManager.initDefaults();
    }
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

    if (musicSliderPtr) musicSliderPtr->setValue((float)tmpMusicVol);
    if (sfxSliderPtr) sfxSliderPtr->setValue((float)tmpSfxVol);
    if (sensSliderPtr) sensSliderPtr->setValue(tmpSensitivity);
    if (uiScaleSliderPtr) uiScaleSliderPtr->setValue(tmpUiScale);
    if (fsBtnPtr) fsBtnPtr->setText(tmpFullscreen ? "ВКЛ" : "ВЫКЛ");
    if (vsyncBtnPtr) vsyncBtnPtr->setText(tmpVsync ? "ВКЛ" : "ВЫКЛ");
}

// Построение иерархии контейнеров
void Menu::initUI() {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());

    // ГЛАВНОЕ МЕНЮ
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

    auto version = std::make_unique<UI::Text>(font, "v0.6b", 24);
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
    mainContainer->addChild(std::move(btnsCont));

    // ВЫБОР УРОВНЯ
    UI::Container* navArea = nullptr;
    levelContainer = createSubMenu("ВЫБОР УРОВНЯ", &cardsArea, &navArea);

    if (cardsArea) {
        cardsArea->setDirection(UI::Container::Direction::Row);
        cardsArea->setContentAlign(UI::Container::ContentAlign::Center);
        cardsArea->setItemAlign(UI::Container::ItemAlign::Center);
        cardsArea->setBackgroundTexture(ResourceManager::get("panel-light"), 64.f);
        cardsArea->setWrap(true);
        cardsArea->setGap(25.f);
        cardsArea->setPadding({ 30.f, 30.f });
        cardsArea->setScrollEnabled(true);

        for (const auto& level : levels) {
            sf::Vector2f cardSize(325.f, 225.f);
            auto card = std::make_unique<UI::Container>(cardSize);
            card->setDirection(UI::Container::Direction::Column);
            card->setItemAlign(UI::Container::ItemAlign::Center);
            card->setPadding({ 10.f, 10.f });
            card->setGap(5.f);
            
            bool unlocked = saveManager.isUnlocked(level.id);
            card->setBackgroundTexture(ResourceManager::get("card"), 12.f);
            if (!unlocked) card->setBackgroundColor(sf::Color(100, 100, 100, 150));

            auto numText = std::make_unique<UI::Text>(font, "УРОВЕНЬ " + std::to_string(level.index + 1), 24);
            numText->setColor(unlocked ? Colors::Theme::TextDark : sf::Color(50, 50, 50));
            card->addChild(std::move(numText));

            auto nameText = std::make_unique<UI::Text>(font, level.name, 26);
            nameText->setColor(unlocked ? Colors::Theme::TextMain : sf::Color(80, 80, 80));
            card->addChild(std::move(nameText));

            if (unlocked) {
                // Рекорд (В самом низу)
                int maxWave = saveManager.getMaxWave(level.id);
                int bestScore = saveManager.getBestScore(level.id);

                auto recHeader = std::make_unique<UI::Text>(font, "РЕКОРД", 20);
                recHeader->setColor(Colors::Theme::TextYellow);
                card->addChild(std::move(recHeader));

                auto waveRec = std::make_unique<UI::Text>(font, "Волна: " + std::to_string(maxWave), 18);
                waveRec->setColor(sf::Color(200, 200, 200));
                card->addChild(std::move(waveRec));

                auto scoreRec = std::make_unique<UI::Text>(font, "Счет: " + std::to_string(bestScore), 18);
                scoreRec->setColor(sf::Color(200, 200, 200));
                card->addChild(std::move(scoreRec));

                // Звезды
                auto starsRow = std::make_unique<UI::Container>(sf::Vector2f(cardSize.x, 30.f));
                starsRow->setDirection(UI::Container::Direction::Row);
                starsRow->setContentAlign(UI::Container::ContentAlign::Center);
                starsRow->setItemAlign(UI::Container::ItemAlign::Center);
                starsRow->setGap(10.f);

                int savedStars = saveManager.getStars(level.id);
                for (int i = 0; i < 3; ++i) {
                    const auto& tex = (i < savedStars) ? ResourceManager::get("icon-star-filled") : ResourceManager::get("icon-star-empty");
                    auto starImg = std::make_unique<UI::Image>(tex, sf::Vector2f(64.f, 64.f));
                    if (i >= savedStars) starImg->setColor(sf::Color(100, 100, 100, 100));
                    starsRow->addChild(std::move(starImg));
                }
                card->addChild(std::move(starsRow));
            } else {
                auto lockText = std::make_unique<UI::Text>(font, "ЗАБЛОКИРОВАНО", 18);
                lockText->setColor(sf::Color::Red);
                card->addChild(std::move(lockText));
            }

            auto clicker = std::make_unique<UI::Button>(font, "", cardSize);
            clicker->setTransparent(true);
            clicker->setFollowsLayout(false);
            clicker->setEnabled(unlocked);
            clicker->setCallback([this, path = level.filePath, area = cardsArea]() {
                if (area && !area->isCurrentlyDragging()) { selectedLevel = path; updateCardsSelection(); }
            });
            card->addChild(std::move(clicker));
            cardsArea->addChild(std::move(card));
        }
    }

    if (navArea) {
        auto startGameBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-play"), font, "ИГРАТЬ", sf::Vector2f(220.f, 60.f), UI::Button::IconPlacement::Right);
        startGameBtn->setBackgroundTextures(&ResourceManager::get("button"), &ResourceManager::get("button-hover"), &ResourceManager::get("button-active"), nullptr, 32.0f);
        startGameBtn->setIconScale({ 0.5f, 0.5f });
        startGameBtn->setCallback([this]() { if (!selectedLevel.empty()) levelChosen = true; });
        startGameBtn->setEnabled(false);
        playBtnPtr = startGameBtn.get();
        navArea->addChild(std::move(startGameBtn));
    }

    // НАСТРОЙКИ
    UI::Container* settingsContent = nullptr;
    UI::Container* settingsNav = nullptr;
    settingsContainer = createSubMenu("НАСТРОЙКИ", &settingsContent, &settingsNav);

    if (settingsContent) {
        settingsContent->setDirection(UI::Container::Direction::Column);
        settingsContent->setContentAlign(UI::Container::ContentAlign::Center);
        settingsContent->setItemAlign(UI::Container::ItemAlign::Center);
        settingsContent->setGap(20.f);
        settingsContent->setPadding({ 20.f, 20.f });
        settingsContent->setScrollEnabled(true);

        auto createRow = [&](const sf::Texture& iconTex, const std::string& label, std::unique_ptr<UI::Widget> control) {
            auto row = std::make_unique<UI::Container>(sf::Vector2f(950.f, 60.f));
            row->setDirection(UI::Container::Direction::Row);
            row->setContentAlign(UI::Container::ContentAlign::Center);
            row->setItemAlign(UI::Container::ItemAlign::Center);
            row->setGap(10.f);
            row->addChild(std::make_unique<UI::Image>(iconTex, sf::Vector2f(48.f, 48.f)));
            auto text = std::make_unique<UI::Text>(font, label, 24, sf::Vector2f(350.f, 60.f));
            text->setColor(Colors::Theme::TextMain);
            row->addChild(std::move(text));
            row->addChild(std::move(control));
            return row;
        };

        auto musicSlider = std::make_unique<UI::Slider>(font, 0.f, 100.f, (float)tmpMusicVol, sf::Vector2f(350.f, 30.f));
        musicSliderPtr = musicSlider.get();
        musicSlider->setCallback([this](float value) { tmpMusicVol = (int)value; });
        settingsContent->addChild(createRow(ResourceManager::get("icon-music"), "ГРОМКОСТЬ МУЗЫКИ", std::move(musicSlider)));

        auto sfxSlider = std::make_unique<UI::Slider>(font, 0.f, 100.f, (float)tmpSfxVol, sf::Vector2f(350.f, 30.f));
        sfxSliderPtr = sfxSlider.get();
        sfxSlider->setCallback([this](float value) { tmpSfxVol = (int)value; });
        settingsContent->addChild(createRow(ResourceManager::get("icon-audio"), "ГРОМКОСТЬ ЗВУКОВ", std::move(sfxSlider)));

        auto sensSlider = std::make_unique<UI::Slider>(font, 0.5f, 3.0f, tmpSensitivity, sf::Vector2f(350.f, 30.f));
        sensSlider->setPrecision(1);
        sensSliderPtr = sensSlider.get();
        sensSlider->setCallback([this](float value) { tmpSensitivity = value; });
        settingsContent->addChild(createRow(ResourceManager::get("icon-sensivity"), "ЧУВСТВИТЕЛЬНОСТЬ", std::move(sensSlider)));

        auto uiScaleSlider = std::make_unique<UI::Slider>(font, 0.6f, 1.6f, tmpUiScale, sf::Vector2f(350.f, 30.f));
        uiScaleSlider->setPrecision(1);
        uiScaleSliderPtr = uiScaleSlider.get();
        uiScaleSlider->setCallback([this](float value) { tmpUiScale = value; });
        settingsContent->addChild(createRow(ResourceManager::get("icon-display"), "МАСШТАБ ИНТЕРФЕЙСА", std::move(uiScaleSlider)));

#ifndef __ANDROID__ 
        auto vsyncBtn = std::make_unique<UI::Button>(font, tmpVsync ? "ВКЛ" : "ВЫКЛ", sf::Vector2f(350.f, 45.f));
        vsyncBtnPtr = vsyncBtn.get();
        vsyncBtn->setBackgroundTextures(&ResourceManager::get("card"), &ResourceManager::get("card-light"), nullptr, nullptr, 12.f);
        vsyncBtn->setCallback([this]() { tmpVsync = !tmpVsync; if (vsyncBtnPtr) vsyncBtnPtr->setText(tmpVsync ? "ВКЛ" : "ВЫКЛ"); });
        settingsContent->addChild(createRow(ResourceManager::get("icon-vsync"), "ВЕРТИКАЛЬНАЯ СИНХР.", std::move(vsyncBtn)));

        auto fsBtn = std::make_unique<UI::Button>(font, tmpFullscreen ? "ВКЛ" : "ВЫКЛ", sf::Vector2f(350.f, 45.f));
        fsBtnPtr = fsBtn.get();
        fsBtn->setBackgroundTextures(&ResourceManager::get("card"), &ResourceManager::get("card-light"), nullptr, nullptr, 32.f);
        fsBtn->setCallback([this]() { tmpFullscreen = !tmpFullscreen; if (fsBtnPtr) fsBtnPtr->setText(tmpFullscreen ? "ВКЛ" : "ВЫКЛ"); });
        settingsContent->addChild(createRow(ResourceManager::get("icon-fullscreen"), "ПОЛНОЭКРАННЫЙ РЕЖИМ", std::move(fsBtn)));
#endif
    }
    
    if (settingsNav) {
        auto saveBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-save"), font, "СОХРАНИТЬ", sf::Vector2f(220.f, 60.f), UI::Button::IconPlacement::Right);
        saveBtn->setBackgroundTextures(&ResourceManager::get("button"), &ResourceManager::get("button-hover"), &ResourceManager::get("button-active"), nullptr, 32.0f);
        saveBtn->setIconScale({ 0.5f, 0.5f });
        saveBtn->setCallback([this]() {
            settings.set<int>("music_volume", tmpMusicVol); settings.set<int>("sfx_volume", tmpSfxVol);
            settings.set<float>("sensitivity", tmpSensitivity); settings.set<float>("ui_scale", tmpUiScale);
            settings.set<bool>("fullscreen", tmpFullscreen); settings.set<bool>("vsync", tmpVsync);
            settings.save();
            window.setVerticalSyncEnabled(tmpVsync);
            updateViewSizes(window.getSize());
            state = MenuState::Main;
        });
        settingsNav->addChild(std::move(saveBtn));
    }

    // УЛУЧШЕНИЯ
    upgradesContainer = createUpgradeMenu();

    resultOverlay = std::make_unique<UI::Container>(winSize);
    resultOverlay->setBackgroundColor(Colors::Theme::Overlay);
}

std::unique_ptr<UI::Container> Menu::createSubMenu(const std::string& title, UI::Container** outContent, UI::Container** outNav) {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());
    auto root = std::make_unique<UI::Container>(winSize);
    root->setDirection(UI::Container::Direction::Column);
    root->setItemAlign(UI::Container::ItemAlign::Center);
    root->setContentAlign(UI::Container::ContentAlign::Center);
    root->setBackgroundTexture(ResourceManager::get("panel"), 64.f);
    root->setPadding({ 20.f, 5.f });
    root->setGap(20.f);

    auto header = std::make_unique<UI::Text>(font, title, 60, sf::Vector2f(winSize.x * 0.9f, 80.f));
    header->setAlignment(UI::Text::Align::Center);
    header->setColor(Colors::Theme::TextMain);
    root->addChild(std::move(header));

    auto content = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 400.f)); 
    content->setDirection(UI::Container::Direction::Column);
    content->setItemAlign(UI::Container::ItemAlign::Center);
    content->setContentAlign(UI::Container::ContentAlign::Center);
    content->setBackgroundTexture(ResourceManager::get("panel-light"), 64.f);
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
    root->setBackgroundTexture(ResourceManager::get("panel"), 64.f);
    root->setPadding({ 20.f, 5.f });
    root->setGap(10.f);

    auto header = std::make_unique<UI::Text>(font, "УЛУЧШЕНИЯ", 60, sf::Vector2f(winSize.x * 0.9f, 80.f));
    header->setAlignment(UI::Text::Align::Center);
    header->setColor(Colors::Theme::TextMain);
    root->addChild(std::move(header));

    auto mainBox = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 400.f));
    mainBox->setDirection(UI::Container::Direction::Column);
    mainBox->setItemAlign(UI::Container::ItemAlign::Center);
    mainBox->setContentAlign(UI::Container::ContentAlign::Center);
    mainBox->setBackgroundTexture(ResourceManager::get("panel-light"), 64.f);
    mainBox->setGap(10.f);

    auto currency = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 75.f));
    currency->setDirection(UI::Container::Direction::Row);
    currency->setContentAlign(UI::Container::ContentAlign::Center);
    currency->setItemAlign(UI::Container::ItemAlign::Center);
    currency->setGap(15.f);
    currency->addChild(std::make_unique<UI::Image>(ResourceManager::get("icon-money"), sf::Vector2f(48.f, 48.f)));
    auto moneyText = std::make_unique<UI::Text>(font, std::to_string(saveManager.getMoney()), 24);
    moneyTextPtr = moneyText.get();
    currency->addChild(std::move(moneyText));
    mainBox->addChild(std::move(currency));

    auto scrollArea = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 300.f));
    scrollArea->setDirection(UI::Container::Direction::Column);
    scrollArea->setItemAlign(UI::Container::ItemAlign::Center);
    scrollArea->setScrollEnabled(true);
    scrollArea->setGap(30.f);

    auto sec1H = std::make_unique<UI::Text>(font, "МОДЕРНИЗАЦИЯ ТУРЕЛЕЙ", 36);
    sec1H->setColor(Colors::Theme::TextYellow);
    scrollArea->addChild(std::move(sec1H));

    auto turretGrid = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.85f, 600.f));
    turretGrid->setDirection(UI::Container::Direction::Row);
    turretGrid->setWrap(true);
    turretGrid->setGap(15.f);
    turretGrid->setContentAlign(UI::Container::ContentAlign::Center);
    turretGrid->setItemAlign(UI::Container::ItemAlign::Center);

    std::vector<std::string> towerTypes = GameData::getTowerNames();
    std::vector<std::string> statNames = { "РАНГ", "АТАКА", "СКОР. АТАКИ", "РАДИУС", "УРОВЕНЬ" };
    upgradeValuePtrs.clear(); upgradeValuePtrs.resize(towerTypes.size());
    upgradeCostPtrs.clear(); upgradeCostPtrs.resize(towerTypes.size());
    upgradeBtnPtrs.clear(); upgradeBtnPtrs.resize(towerTypes.size());

    for (size_t t = 0; t < towerTypes.size(); ++t) {
        auto towerCard = std::make_unique<UI::Container>(sf::Vector2f(450.f, 260.f));
        towerCard->setDirection(UI::Container::Direction::Column);
        towerCard->setItemAlign(UI::Container::ItemAlign::Center);
        towerCard->setPadding({ 10.f, 8.f });
        towerCard->setGap(5.f);
        towerCard->setBackgroundTexture(ResourceManager::get("card"), 12.f);
        
        std::string upperName = towerTypes[t];
        std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
        auto tName = std::make_unique<UI::Text>(font, upperName, 28);
        tName->setColor(Colors::Theme::TextMain);
        towerCard->addChild(std::move(tName));

        for (size_t s = 0; s < statNames.size(); ++s) {
            auto statRow = std::make_unique<UI::Container>(sf::Vector2f(400.f, 36.f));
            statRow->setDirection(UI::Container::Direction::Row);
            statRow->setContentAlign(UI::Container::ContentAlign::Center);
            statRow->setItemAlign(UI::Container::ItemAlign::Center);
            statRow->setGap(5.f);
            
            auto sName = std::make_unique<UI::Text>(font, statNames[s], 18, sf::Vector2f(170.f, 32.f));
            sName->setAlignment(UI::Text::Align::Left);
            statRow->addChild(std::move(sName));

            auto sVal = std::make_unique<UI::Text>(font, "0", 18, sf::Vector2f(70.f, 32.f));
            sVal->setAlignment(UI::Text::Align::Left);
            upgradeValuePtrs[t].push_back(sVal.get());
            statRow->addChild(std::move(sVal));

            auto sCost = std::make_unique<UI::Text>(font, "0", 18, sf::Vector2f(80.f, 32.f));
            sCost->setAlignment(UI::Text::Align::Left);
            upgradeCostPtrs[t].push_back(sCost.get());
            statRow->addChild(std::move(sCost));

            auto uBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-upgrade2"), sf::Vector2f(50.f, 32.f));
            uBtn->setBackgroundTextures(&ResourceManager::get("button-flat"), &ResourceManager::get("button-flat-hover"), &ResourceManager::get("button-flat"), &ResourceManager::get("button-flat-disabled"), 16.0f);
            uBtn->setIconScale({ 32.f/96.f, 32.f/96.f });
            auto tType = towerTypes[t];
            UpgradeManager* um = &upgradeManager;
            SaveManager* sm = &saveManager;
            uBtn->setCallback([um, sm, tType, sIndex = s]() {
                std::string keys[] = {"rank", "damage", "firerate", "range", "level"};
                if (um->isStatAtLimit(tType, keys[sIndex])) return;
                int cost = um->getUpgradeCost(tType, (int)sIndex);
                if (sm->spendMoney(cost)) {
                    if (sIndex == 0) um->upgradeRank(tType);
                    else if (sIndex == 1) um->upgradeDamage(tType, 0.1f);
                    else if (sIndex == 2) um->upgradeFirerate(tType, 0.1f);
                    else if (sIndex == 3) um->upgradeRange(tType, 0.1f);
                    else if (sIndex == 4) um->upgradeMaxLevel(tType);
                }
            });
            upgradeBtnPtrs[t].push_back(uBtn.get());
            statRow->addChild(std::move(uBtn));
            towerCard->addChild(std::move(statRow));
        }
        turretGrid->addChild(std::move(towerCard));
    }
    scrollArea->addChild(std::move(turretGrid));

    auto sec2H = std::make_unique<UI::Text>(font, "СТРАТЕГИЧЕСКИЙ ОТДЕЛ", 36);
    sec2H->setColor(Colors::Theme::TextYellow);
    scrollArea->addChild(std::move(sec2H));

    auto stratCont = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.85f, 250.f));
    stratCont->setDirection(UI::Container::Direction::Column);
    stratCont->setItemAlign(UI::Container::ItemAlign::Center);
    stratCont->setGap(10.f);

    struct MetaUp { std::string name, icon; int costStep; };
    std::vector<MetaUp> metaUps = { { "НАЧАЛЬНЫЙ КАПИТАЛ", "icon-coins", 500 }, { "ДОХОДНОСТЬ", "icon-chart", 750 }, { "ЖИЗНИ БАЗЫ", "icon-heart", 400 } };
    metaValuePtrs.clear(); metaCostPtrs.clear(); metaBtnPtrs.clear();

    for (int i = 0; i < 3; ++i) {
        auto row = std::make_unique<UI::Container>(sf::Vector2f(800.f, 60.f));
        row->setDirection(UI::Container::Direction::Row);
        row->setContentAlign(UI::Container::ContentAlign::Center);
        row->setItemAlign(UI::Container::ItemAlign::Center);
        row->setGap(20.f);
        row->setBackgroundTexture(ResourceManager::get("card"), 8.f);
        row->addChild(std::make_unique<UI::Image>(ResourceManager::get(metaUps[i].icon), sf::Vector2f(40.f, 40.f)));
        auto nLabel = std::make_unique<UI::Text>(font, metaUps[i].name, 22, sf::Vector2f(300.f, 40.f));
        nLabel->setAlignment(UI::Text::Align::Left);
        row->addChild(std::move(nLabel));
        auto vText = std::make_unique<UI::Text>(font, "Ур. 0", 22, sf::Vector2f(100.f, 40.f));
        metaValuePtrs.push_back(vText.get());
        row->addChild(std::move(vText));
        auto cText = std::make_unique<UI::Text>(font, "500", 22, sf::Vector2f(120.f, 40.f));
        cText->setColor(Colors::Theme::TextYellow);
        metaCostPtrs.push_back(cText.get());
        row->addChild(std::move(cText));
        auto uBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-upgrade2"), sf::Vector2f(60.f, 40.f));
        uBtn->setBackgroundTextures(&ResourceManager::get("button-flat"), &ResourceManager::get("button-flat-hover"), &ResourceManager::get("button-flat"), &ResourceManager::get("button-flat-disabled"), 16.0f);
        uBtn->setIconScale({ 0.4f, 0.4f });
        SaveManager* sm = &saveManager;
        uBtn->setCallback([sm, i, costStep = metaUps[i].costStep]() {
            int curLvl = (i == 0 ? sm->getGlobalCoinsLvl() : (i == 1 ? sm->getGlobalMoneyLvl() : sm->getGlobalBaseHpLvl()));
            int cost = (curLvl + 1) * costStep;
            if (sm->spendMoney(cost)) {
                if (i == 0) sm->setGlobalCoinsLvl(curLvl + 1); else if (i == 1) sm->setGlobalMoneyLvl(curLvl + 1); else sm->setGlobalBaseHpLvl(curLvl + 1);
                sm->save();
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
        card->setBackgroundTexture(ResourceManager::get(levels[i].filePath == selectedLevel ? "card-light" : "card"), 12.f);
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
            for (size_t s = 0; s < upgradeValuePtrs[t].size(); ++s) {
                float curVal = 0.f; int curStep = 0;
                std::string keys[] = {"rank", "damage", "firerate", "range", "level"};
                if (s == 0) curStep = upgradeManager.getRank(tTypes[t]);
                else if (s == 1) curVal = upgradeManager.getDamage(tTypes[t]);
                else if (s == 2) curVal = upgradeManager.getFirerate(tTypes[t]);
                else if (s == 3) curVal = upgradeManager.getRange(tTypes[t]);
                else if (s == 4) curStep = upgradeManager.getLevel(tTypes[t]);
                
                std::string vStr;
                if (s == 0 || s == 4) vStr = std::to_string(curStep);
                else if (s == 2) {
                    vStr = std::to_string(curVal);
                    if (vStr.find('.') != std::string::npos) vStr = vStr.substr(0, vStr.find('.') + 3);
                } else vStr = std::to_string((int)curVal);
                
                upgradeValuePtrs[t][s]->setText(vStr);
                int cost = upgradeManager.getUpgradeCost(tTypes[t], (int)s);
                bool atLimit = upgradeManager.isStatAtLimit(tTypes[t], keys[s]);
                upgradeCostPtrs[t][s]->setText(atLimit ? "MAX" : std::to_string(cost));
                upgradeCostPtrs[t][s]->setColor(atLimit ? sf::Color(150,150,150) : (saveManager.getMoney() >= cost ? Colors::Theme::TextGreen : Colors::Theme::TextRed));
                upgradeBtnPtrs[t][s]->setEnabled(!atLimit);
            }
        }
        int mLevels[] = { saveManager.getGlobalCoinsLvl(), saveManager.getGlobalMoneyLvl(), saveManager.getGlobalBaseHpLvl() };
        int cSteps[] = { 500, 750, 400 };
        for (int i = 0; i < 3; ++i) {
            if (i < (int)metaValuePtrs.size()) metaValuePtrs[i]->setText("Ур. " + std::to_string(mLevels[i]));
            int cost = (mLevels[i] + 1) * cSteps[i];
            if (i < (int)metaCostPtrs.size()) {
                metaCostPtrs[i]->setText(std::to_string(cost));
                metaCostPtrs[i]->setColor(saveManager.getMoney() >= cost ? Colors::Theme::TextGreen : Colors::Theme::TextRed);
            }
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
        if (headerContPtr) headerContPtr->setSize(sf::Vector2f(rootSize.x * 0.9f, 125.f));
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
                        if (asContainer->getChildrenCount() >= 2 && cont == upgradesContainer) {
                            auto *cur = asContainer->getChild(0), *cards = asContainer->getChild(1);
                            if (cur) cur->setSize(sf::Vector2f(targetWidth, cur->getSize().y));
                            if (cards) cards->setSize(sf::Vector2f(targetWidth, child->getSize().y - (cur ? cur->getSize().y : 0.f) - 10.f));
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
    const std::string dirPath = "data/levels/";
    if (!fs::exists(dirPath)) return;
    std::vector<fs::path> mapPaths;
    for (const auto& entry : fs::directory_iterator(dirPath)) if (entry.is_regular_file() && entry.path().extension() == ".map") mapPaths.push_back(entry.path());
    std::sort(mapPaths.begin(), mapPaths.end());
    for (int i = 0; i < (int)mapPaths.size(); ++i) {
        LevelInfo info; info.filePath = mapPaths[i].string(); info.id = mapPaths[i].stem().string(); info.name = readLevelName(info.filePath); info.index = i;
        levels.push_back(info);
    }
}

std::string Menu::readLevelName(const std::string& path) const {
    auto content = readFile(path); if (!content || content->empty()) return "Безымянный";
    size_t pos = content->find("name="); if (pos == std::string::npos) return "Безымянный";
    size_t start = pos + 5, end = content->find("\n", start);
    std::string name = (end == std::string::npos) ? content->substr(start) : content->substr(start, end - start);
    if (!name.empty() && name.back() == '\r') name.pop_back();
    return name.empty() ? "Безымянный" : name;
}

bool Menu::isLevelChosen() const { return levelChosen; }
std::string Menu::getChosenLevel() const { return selectedLevel; }
void Menu::resetChoice() { levelChosen = false; selectedLevel = ""; updateCardsSelection(); }
void Menu::resetLastResult() { lastResult = SessionResult::None; }
bool Menu::consumesWindowRecreationRequest() { bool req = windowRecreationRequired; windowRecreationRequired = false; return req; }
void Menu::notifyResult(SessionResult result, const std::string& levelPath) { lastResult = result; lastLevelPath = levelPath; }
void Menu::cleanup() {
    mainContainer.reset(); levelContainer.reset(); settingsContainer.reset(); upgradesContainer.reset(); resultOverlay.reset();
    upgradeValuePtrs.clear(); upgradeCostPtrs.clear(); upgradeBtnPtrs.clear(); moneyTextPtr = nullptr;
}
int Menu::getMoney() const { return saveManager.getMoney(); }
UpgradeManager& Menu::getUpgradeManager() { return upgradeManager; }
