#include "Menu.hpp"
#include "ResourceManager.hpp"
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


namespace fs = std::filesystem;

// Инициализирует системы и строит интерфейс
Menu::Menu(sf::RenderWindow& window, SettingsManager& settings) 
    : window(window), settings(settings) {
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
    mainContainer->setPadding({ 20.f, 20.f });
    mainContainer->setGap(30.f);
    mainContainer->setDrawOutline(true);

    auto headerCont = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.8f, 125.f));
    headerCont->setDirection(UI::Container::Direction::Column);
    headerCont->setContentAlign(UI::Container::ContentAlign::Center);
    headerCont->setItemAlign(UI::Container::ItemAlign::Center);
    headerCont->setGap(20.f); 
    headerCont->setDrawOutline(true);
    headerContPtr = headerCont.get();

    auto title = std::make_unique<UI::Text>(font, "TOWER DEFENCE", 96);
    title->setColor(Colors::Theme::TextMain);
    titleTextPtr = title.get();
    headerCont->addChild(std::move(title));

    auto version = std::make_unique<UI::Text>(font, "v0.4a", 24);
    version->setColor(sf::Color(180, 180, 180));
    headerCont->addChild(std::move(version));
    mainContainer->addChild(std::move(headerCont));

    auto btnsCont = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.8f, 320.f));
    btnsCont->setDirection(UI::Container::Direction::Column);
    btnsCont->setContentAlign(UI::Container::ContentAlign::Center);
    btnsCont->setItemAlign(UI::Container::ItemAlign::Center);
    btnsCont->setGap(15.f);
    btnsCont->setDrawOutline(true);
    btnsContPtr = btnsCont.get();

    sf::Vector2f btnSize(275.f, 60.f);
    auto playBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-play"), font, "ИГРАТЬ", btnSize, UI::IconPlacement::Left);
    playBtn->setIconScale({ 0.5f, 0.5f });
    playBtn->setCallback([this]() { state = MenuState::LevelSelect; });
    playBtn->setTextSize(20);
    btnsCont->addChild(std::move(playBtn));

    auto upgBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-upgrades"), font, "УЛУЧШЕНИЯ", btnSize, UI::IconPlacement::Left);
    upgBtn->setIconScale({ 0.5f, 0.5f });
    upgBtn->setCallback([this]() { state = MenuState::Upgrades; });
    upgBtn->setTextSize(20);
    btnsCont->addChild(std::move(upgBtn));

    auto setBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-settings"), font, "НАСТРОЙКИ", btnSize, UI::IconPlacement::Left);
    setBtn->setIconScale({ 0.5f, 0.5f });
    setBtn->setCallback([this]() { syncSettingsToTmp(); state = MenuState::Settings; });
    setBtn->setTextSize(20);
    btnsCont->addChild(std::move(setBtn));

    auto exitBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-exit"), font, "ВЫХОД", btnSize, UI::IconPlacement::Left);
    exitBtn->setIconScale({ 0.5f, 0.5f });
    exitBtn->setCallback([this]() { window.close(); });
    exitBtn->setTextSize(20);
    btnsCont->addChild(std::move(exitBtn));
    mainContainer->addChild(std::move(btnsCont));

    // экран выбора уровня
    UI::Container* navArea = nullptr;
    levelContainer = createSubMenu("ВЫБОР УРОВНЯ", &cardsArea, &navArea);
    
    if (cardsArea) {
        cardsArea->setDirection(UI::Container::Direction::Row);
        cardsArea->setContentAlign(UI::Container::ContentAlign::Center);
        cardsArea->setWrap(true);
        cardsArea->setGap(25.f);
        cardsArea->setPadding({ 20.f, 20.f });
        cardsArea->setDrawOutline(true);
        cardsArea->setScrollEnabled(true);

        for (const auto& level : levels) {
            sf::Vector2f cardSize(260.f, 140.f);
            auto card = std::make_unique<UI::Container>(cardSize);
            card->setDirection(UI::Container::Direction::Column);
            card->setContentAlign(UI::Container::ContentAlign::Start);
            card->setItemAlign(UI::Container::ItemAlign::Center);
            card->setDrawBackground(true);
            card->setBackgroundColor(sf::Color(45, 45, 45));
            card->setPadding({ 5.f, 5.f });
            card->setDrawOutline(true);

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

            auto nameBlock = std::make_unique<UI::Container>(sf::Vector2f(cardSize.x, cardSize.y - 55.f));
            nameBlock->setDirection(UI::Container::Direction::Column);
            nameBlock->setContentAlign(UI::Container::ContentAlign::Center);
            nameBlock->setItemAlign(UI::Container::ItemAlign::Center);
            nameBlock->setDrawOutline(true);
            auto nameText = std::make_unique<UI::Text>(font, level.name, 20);
            nameText->setColor(Colors::Theme::TextMain);
            nameBlock->addChild(std::move(nameText));
            card->addChild(std::move(nameBlock));

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
        auto startGameBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-play"), font, "ИГРАТЬ", sf::Vector2f(220.f, 60.f),  UI::IconPlacement::Right);
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
    
    if (settingsContent) {
        settingsContent->setGap(20.f);
        settingsContent->setPadding({ 20.f, 20.f });
        settingsContent->setItemAlign(UI::Container::ItemAlign::Center);
        settingsContent->setContentAlign(UI::Container::ContentAlign::Center);

        // лямбда для создания строки меню с иконкой, заголовком и контролом
        auto createRow = [&](const sf::Texture& iconTex, const std::string& label, std::unique_ptr<UI::Widget> control, std::unique_ptr<UI::Text> valueText = nullptr) {
            auto row = std::make_unique<UI::Container>(sf::Vector2f(950.f, 60.f));
            row->setDirection(UI::Container::Direction::Row);
            row->setContentAlign(UI::Container::ContentAlign::Center);
            row->setItemAlign(UI::Container::ItemAlign::Center);
            row->setGap(30.f);

            // Добавление иконки настройки
            auto icon = std::make_unique<UI::Image>(iconTex, sf::Vector2f(48.f, 48.f));
            row->addChild(std::move(icon));

            // Контейнер для текста заголовка (фиксированная ширина для выравнивания)
            auto textCont = std::make_unique<UI::Container>(sf::Vector2f(350.f, 60.f));
            textCont->setContentAlign(UI::Container::ContentAlign::Center);
            textCont->setItemAlign(UI::Container::ItemAlign::Start);
            auto text = std::make_unique<UI::Text>(font, label, 24);
            text->setColor(Colors::Theme::TextMain);
            textCont->addChild(std::move(text));

            row->addChild(std::move(textCont));
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

        // Настройка: Масштаб интерфейса (выбор иконки и границ в зависимости от платформы)
#ifdef __ANDROID__
        float minScale = 0.7f;
        float maxScale = 1.5f;
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
        vsyncBtn->setCallback([this]() {
            tmpVsync = !tmpVsync;
            if (vsyncBtnPtr) vsyncBtnPtr->setText(tmpVsync ? "ВКЛ" : "ВЫКЛ");
        });
        settingsContent->addChild(createRow(ResourceManager::get("icon-vsync"), "ВЕРТИКАЛЬНАЯ СИНХР.", std::move(vsyncBtn)));

        // Настройка: Полноэкранный режим
        std::string fsLabel = tmpFullscreen ? "ВКЛ" : "ВЫКЛ";
        auto fsBtn = std::make_unique<UI::Button>(font, fsLabel, sf::Vector2f(350.f, 45.f));
        fsBtnPtr = fsBtn.get();
        fsBtn->setCallback([this]() {
            tmpFullscreen = !tmpFullscreen;
            if (fsBtnPtr) fsBtnPtr->setText(tmpFullscreen ? "ВКЛ" : "ВЫКЛ");
        });
        
        settingsContent->addChild(createRow(ResourceManager::get("icon-fullscreen"), "ПОЛНОЭКРАННЫЙ РЕЖИМ", std::move(fsBtn)));
#endif
    }
    
    if (settingsNav) {
        auto saveBtn = std::make_unique<UI::Button>(ResourceManager::get("icon-save"), font, "СОХРАНИТЬ", sf::Vector2f(220.f, 60.f), UI::IconPlacement::Right);
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

    upgradesContainer = createSubMenu("УЛУЧШЕНИЯ", nullptr);

    resultOverlay = std::make_unique<UI::Container>(winSize);
    resultOverlay->setDirection(UI::Container::Direction::Column);
    resultOverlay->setContentAlign(UI::Container::ContentAlign::Center);
    resultOverlay->setItemAlign(UI::Container::ItemAlign::Center);
    resultOverlay->setBackgroundColor(sf::Color(0, 0, 0, 200));
    resultOverlay->setDrawBackground(true);
    resultOverlay->setDrawOutline(true);
}

// Создание вложенного меню
std::unique_ptr<UI::Container> Menu::createSubMenu(const std::string& title, UI::Container** outContent, UI::Container** outNav) {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());

    auto root = std::make_unique<UI::Container>(winSize);
    root->setDirection(UI::Container::Direction::Column);
    root->setContentAlign(UI::Container::ContentAlign::Start); 
    root->setItemAlign(UI::Container::ItemAlign::Center);
    root->setPadding({ 20.f, 5.f });
    root->setGap(20.f);
    root->setDrawOutline(true);

    auto header = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 80.f)); 
    header->setDirection(UI::Container::Direction::Column);
    header->setContentAlign(UI::Container::ContentAlign::Center);
    header->setItemAlign(UI::Container::ItemAlign::Center);
    header->setDrawOutline(true);
    auto head = std::make_unique<UI::Text>(font, title, 60); 
    head->setColor(Colors::Theme::TextMain);
    header->addChild(std::move(head));
    root->addChild(std::move(header));

    auto content = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 400.f)); 
    content->setDirection(UI::Container::Direction::Column);
    content->setContentAlign(UI::Container::ContentAlign::Center); 
    content->setItemAlign(UI::Container::ItemAlign::Center);
    content->setDrawOutline(true);
    if (outContent) *outContent = content.get();
    root->addChild(std::move(content));

    auto nav = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.9f, 80.f));
    nav->setDirection(UI::Container::Direction::Row);
    nav->setContentAlign(UI::Container::ContentAlign::Center);
    nav->setGap(30.f);
    nav->setDrawOutline(true);
    auto back = std::make_unique<UI::Button>(ResourceManager::get("icon-back"), font, "НАЗАД", sf::Vector2f(220.f, 60.f), UI::IconPlacement::Left);
    back->setIconScale({ 0.5f, 0.5f });
    back->setTextSize(20);
    back->setCallback([this]() { state = MenuState::Main; selectedLevel = ""; updateCardsSelection(); });
    nav->addChild(std::move(back));
    if (outNav) *outNav = nav.get();
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
            card->setBackgroundColor(sf::Color(80, 80, 80));
            card->setDrawOutline(true);
        } else {
            card->setBackgroundColor(sf::Color(45, 45, 45));
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
    window.clear(sf::Color(25, 25, 30));
    window.setView(uiView);
    UI::Container* current = nullptr;
    switch (state) {
        case MenuState::Main: current = mainContainer.get(); break;
        case MenuState::LevelSelect: current = levelContainer.get(); break;
        case MenuState::Settings: current = settingsContainer.get(); break;
        case MenuState::Upgrades: current = upgradesContainer.get(); break;
    }
    if (current) current->render(window);
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

    sf::Vector2f rootSize(uiW * 0.85f, uiH * 0.95f);
    sf::Vector2f rootPos((uiW - rootSize.x) / 2.f, (uiH - rootSize.y) / 2.f);

    if (mainContainer) {
        if (headerContPtr) headerContPtr->setSize(sf::Vector2f(rootSize.x * 0.9f, 220.f));
        if (btnsContPtr) btnsContPtr->setSize(sf::Vector2f(rootSize.x * 0.6f, 320.f));
        if (titleTextPtr) titleTextPtr->setMaxWidth(rootSize.x * 0.8f);
        mainContainer->setSize(rootSize);
        mainContainer->setPosition(rootPos);
        mainContainer->rebuild(); 
    }

    auto updateSub = [&](std::unique_ptr<UI::Container>& cont) {
        if (cont) {
            float headerH = 80.f;  
            float navH = 80.f;  
            float gap = 20.f;   
            float p = 5.f;    
            float contentH = rootSize.y - headerH - navH - gap * 2.f - p * 2.f;

            for (size_t i = 0; i < cont->getChildrenCount(); ++i) {
                auto* child = dynamic_cast<UI::Container*>(cont->getChild(i));
                if (child) {
                    if (i == 1) child->setSize(sf::Vector2f(rootSize.x * 0.95f, contentH));
                    else child->setSize(sf::Vector2f(rootSize.x * 0.95f, child->getSize().y));
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
    // логика для android через ndk asset manager
    ANativeActivity* activity = sf::getNativeActivity();
    AAssetDir* assetDir = AAssetManager_openDir(activity->assetManager, "levels");
    const char* fileName = nullptr;

    while ((fileName = AAssetDir_getNextFileName(assetDir)) != nullptr) {
        std::string sName = fileName;
        if (sName.size() > 4 && sName.substr(sName.size() - 4) == ".map") {
            std::string fullPath = "levels/" + sName;
            levels.push_back({ fullPath, readLevelName(fullPath), (int)levels.size() });
        }
    }
    AAssetDir_close(assetDir);

    // сортировка списка уровней по пути
    std::sort(levels.begin(), levels.end(), [](const LevelInfo& a, const LevelInfo& b) {
        return a.filePath < b.filePath;
    });

#else
    // логика для персональных компьютеров
    const std::string dirPath = "data/levels/";

    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        LOGE("[Menu]: Папка уровней не найдена: %s", dirPath.c_str());
        return;
    }

    std::vector<fs::path> mapFiles;

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".map") {
            mapFiles.push_back(entry.path());
        }
    }
    std::sort(mapFiles.begin(), mapFiles.end());

    for (int i = 0; i < (int)mapFiles.size(); ++i) {
        std::string fullPath = mapFiles[i].string();
        levels.push_back({ fullPath, readLevelName(fullPath), i });
    }
#endif
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
    // удаление символа возврата каретки
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
    text->setColor((result == SessionResult::Win) ? sf::Color::Green : sf::Color::Red);
    resultOverlay->addChild(std::move(text));
    auto back = std::make_unique<UI::Button>(font, "В МЕНЮ", sf::Vector2f(200.f, 60.f));
    back->setCallback([this]() { lastResult = SessionResult::None; });
    resultOverlay->addChild(std::move(back));
}

// Очистка ресурсов интерфейса
void Menu::cleanup() {
    // Полное уничтожение контейнеров для освобождения ресурсов (спрайтов/текстур)
    if (mainContainer) mainContainer.reset();
    if (levelContainer) levelContainer.reset();
    if (settingsContainer) settingsContainer.reset();
    if (upgradesContainer) upgradesContainer.reset();
    if (resultOverlay) resultOverlay.reset();
}
