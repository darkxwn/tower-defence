#include "Menu.hpp"
#include "ResourceManager.hpp"
#include "utils/FileReader.hpp"
#include "Colors.hpp"
#include <filesystem>

namespace fs = std::filesystem;

// Конструктор инициализирует системы и строит интерфейс
Menu::Menu(sf::RenderWindow& window, SettingsManager& settings) 
    : window(window), settings(settings) {
    scanLevels();
    initUI();
    updateViewSizes(window.getSize());
}

// Построение иерархии контейнеров для каждого экрана
void Menu::initUI() {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());

    // ГЛАВНОЕ МЕНЮ
    mainContainer = std::make_unique<UI::Container>(winSize);
    mainContainer->setDirection(UI::Container::Direction::Column);
    mainContainer->setContentAlign(UI::Container::ContentAlign::Center);
    mainContainer->setItemAlign(UI::Container::ItemAlign::Center);
    mainContainer->setPadding({ 20.f, 20.f });
    mainContainer->setGap(60.f);
    mainContainer->setDrawOutline(true);

    auto headerCont = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.8f, 200.f));
    headerCont->setDirection(UI::Container::Direction::Column);
    headerCont->setContentAlign(UI::Container::ContentAlign::Center);
    headerCont->setItemAlign(UI::Container::ItemAlign::Center);
    headerCont->setGap(20.f);
    headerCont->setDrawOutline(true);
    headerContPtr = headerCont.get();

    auto title = std::make_unique<UI::Text>(font, "TOWER DEFENCE", 100);
    title->setColor(sf::Color::White);
    titleTextPtr = title.get();
    headerCont->addChild(std::move(title));

    auto version = std::make_unique<UI::Text>(font, "v0.4a", 24);
    version->setColor(sf::Color(180, 180, 180));
    headerCont->addChild(std::move(version));
    mainContainer->addChild(std::move(headerCont));

    auto btnsCont = std::make_unique<UI::Container>(sf::Vector2f(winSize.x * 0.5f, 320.f));
    btnsCont->setDirection(UI::Container::Direction::Column);
    btnsCont->setContentAlign(UI::Container::ContentAlign::Center);
    btnsCont->setItemAlign(UI::Container::ItemAlign::Center);
    btnsCont->setGap(15.f);
    btnsCont->setDrawOutline(true);
    btnsContPtr = btnsCont.get();

    sf::Vector2f btnSize(300.f, 60.f);
    auto playBtn = std::make_unique<UI::Button>(font, "ИГРАТЬ", btnSize);
    playBtn->setCallback([this]() { state = MenuState::LevelSelect; });
    btnsCont->addChild(std::move(playBtn));

    auto upgBtn = std::make_unique<UI::Button>(font, "УЛУЧШЕНИЯ", btnSize);
    upgBtn->setCallback([this]() { state = MenuState::Upgrades; });
    btnsCont->addChild(std::move(upgBtn));

    auto setBtn = std::make_unique<UI::Button>(font, "НАСТРОЙКИ", btnSize);
    setBtn->setCallback([this]() { state = MenuState::Settings; });
    btnsCont->addChild(std::move(setBtn));

    auto exitBtn = std::make_unique<UI::Button>(font, "ВЫХОД", btnSize);
    exitBtn->setCallback([this]() { window.close(); });
    btnsCont->addChild(std::move(exitBtn));
    mainContainer->addChild(std::move(btnsCont));

    // ЭКРАН ВЫБОРА УРОВНЯ
    UI::Container* navArea = nullptr;
    levelContainer = createSubMenu("ВЫБОР УРОВНЯ", &cardsArea, &navArea);
    
    if (cardsArea) {
        cardsArea->setDirection(UI::Container::Direction::Row);
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
            card->setPadding({ 10.f, 10.f });
            card->setDrawOutline(true);

            auto numBlock = std::make_unique<UI::Container>(sf::Vector2f(cardSize.x, 45.f));
            numBlock->setDirection(UI::Container::Direction::Column);
            numBlock->setContentAlign(UI::Container::ContentAlign::Center);
            numBlock->setItemAlign(UI::Container::ItemAlign::Center);
            auto numText = std::make_unique<UI::Text>(font, "УРОВЕНЬ " + std::to_string(level.index + 1), 26);
            numText->setColor(sf::Color(160, 160, 160));
            numBlock->addChild(std::move(numText));
            card->addChild(std::move(numBlock));

            auto nameBlock = std::make_unique<UI::Container>(sf::Vector2f(cardSize.x, cardSize.y - 55.f));
            nameBlock->setDirection(UI::Container::Direction::Column);
            nameBlock->setContentAlign(UI::Container::ContentAlign::Center);
            nameBlock->setItemAlign(UI::Container::ItemAlign::Center);
            nameBlock->setDrawOutline(true);
            auto nameText = std::make_unique<UI::Text>(font, level.name, 20);
            nameText->setColor(sf::Color::White);
            nameBlock->addChild(std::move(nameText));
            card->addChild(std::move(nameBlock));

            auto clicker = std::make_unique<UI::Button>(font, "", cardSize);
            clicker->setTransparent(true);
            clicker->setFollowsLayout(false);
            clicker->setCallback([this, path = level.filePath]() {
                selectedLevel = path;
                updateCardsSelection();
            });
            card->addChild(std::move(clicker));
            cardsArea->addChild(std::move(card));
        }
    }

    if (navArea) {
        auto startGameBtn = std::make_unique<UI::Button>(font, "ИГРАТЬ", sf::Vector2f(220.f, 60.f));
        startGameBtn->setCallback([this]() { if (!selectedLevel.empty()) levelChosen = true; });
        startGameBtn->setEnabled(false);
        playBtnPtr = startGameBtn.get();
        navArea->addChild(std::move(startGameBtn));
    }

    settingsContainer = createSubMenu("НАСТРОЙКИ", nullptr);
    upgradesContainer = createSubMenu("УЛУЧШЕНИЯ", nullptr);

    resultOverlay = std::make_unique<UI::Container>(winSize);
    resultOverlay->setDirection(UI::Container::Direction::Column);
    resultOverlay->setContentAlign(UI::Container::ContentAlign::Center);
    resultOverlay->setItemAlign(UI::Container::ItemAlign::Center);
    resultOverlay->setBackgroundColor(sf::Color(0, 0, 0, 200));
    resultOverlay->setDrawBackground(true);
    resultOverlay->setDrawOutline(true);
}

// Создание базового экрана подменю (Верхний отступ уменьшен до 5)
std::unique_ptr<UI::Container> Menu::createSubMenu(const std::string& title, UI::Container** outContent, UI::Container** outNav) {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = sf::Vector2f(window.getSize());

    auto root = std::make_unique<UI::Container>(winSize);
    root->setDirection(UI::Container::Direction::Column);
    root->setContentAlign(UI::Container::ContentAlign::Start); 
    root->setItemAlign(UI::Container::ItemAlign::Center);
    root->setPadding({ 20.f, 20.f }); // Уменьшен верхний/нижний отступ до 5 пикселей
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
    auto back = std::make_unique<UI::Button>(font, "НАЗАД", sf::Vector2f(220.f, 60.f));
    back->setCallback([this]() { state = MenuState::Main; selectedLevel = ""; updateCardsSelection(); });
    nav->addChild(std::move(back));
    if (outNav) *outNav = nav.get();
    root->addChild(std::move(nav));

    return root;
}

void Menu::updateCardsSelection() {
    if (!cardsArea) return;
    for (size_t i = 0; i < cardsArea->getChildrenCount(); ++i) {
        auto* card = static_cast<UI::Container*>(cardsArea->getChild(i));
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

void Menu::updateViewSizes(sf::Vector2u windowSize) {
    float sw = static_cast<float>(windowSize.x);
    float sh = static_cast<float>(windowSize.y);
    try { uiScale = settings.getFloat("ui_scale"); } catch (...) { uiScale = 1.0f; }
    if (uiScale <= 0.1f) uiScale = 1.0f;

    float uiH = sh / uiScale;
    float uiW = uiH * (sw / sh);
    uiView = sf::View(sf::FloatRect({ 0.f, 0.f }, { uiW, uiH }));

    sf::Vector2f rootSize(uiW * 0.85f, uiH * 0.95f);
    sf::Vector2f rootPos((uiW - rootSize.x) / 2.f, (uiH - rootSize.y) / 2.f);

    if (mainContainer) {
        if (headerContPtr) headerContPtr->setSize(sf::Vector2f(rootSize.x * 0.9f, 200.f));
        if (btnsContPtr) btnsContPtr->setSize(sf::Vector2f(rootSize.x * 0.6f, 320.f));
        if (titleTextPtr) titleTextPtr->setMaxWidth(rootSize.x * 0.8f);
        mainContainer->setSize(rootSize);
        mainContainer->setPosition(rootPos);
        mainContainer->rebuild(); 
    }

    auto updateSub = [&](std::unique_ptr<UI::Container>& cont) {
        if (cont) {
            float hH = 80.f;  // Новый компактный заголовок
            float nH = 80.f;  
            float g = 30.f;   
            float p = 5.f;    // Новый отступ
            float contentH = rootSize.y - hH - nH - g * 2.f - p * 2.f;

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

void Menu::scanLevels() {
    levels.clear();
    std::string path = "data/levels";
    if (!fs::exists(path)) return;
    int idx = 0;
    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.path().extension() == ".map") {
            LevelInfo info;
            info.filePath = entry.path().string();
            info.name = readLevelName(info.filePath);
            info.index = idx++;
            levels.push_back(info);
        }
    }
}

std::string Menu::readLevelName(const std::string& path) const {
    auto content = readFile(path);
    if (!content || content->empty()) return "Безымянный";
    std::string search = "name=";
    size_t pos = content->find(search);
    if (pos == std::string::npos) return "Безымянный";
    size_t start = pos + search.length();
    size_t end = content->find("\n", start);
    std::string name = (end == std::string::npos) ? content->substr(start) : content->substr(start, end - start);
    if (!name.empty() && name.back() == '\r') name.pop_back(); // Исправлен символ
    if (!name.empty() && name.back() == '\r') name.pop_back();
    return name.empty() ? "Безымянный" : name;
}

bool Menu::isLevelChosen() const { return levelChosen; }
std::string Menu::getChosenLevel() const { return selectedLevel; }
void Menu::resetChoice() { levelChosen = false; selectedLevel = ""; updateCardsSelection(); }
bool Menu::consumesWindowRecreationRequest() { bool req = windowRecreationRequired; windowRecreationRequired = false; return req; }

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

void Menu::cleanup() {
    if (mainContainer) mainContainer->clearChildren();
    if (levelContainer) levelContainer->clearChildren();
    if (settingsContainer) settingsContainer->clearChildren();
    if (upgradesContainer) upgradesContainer->clearChildren();
    if (resultOverlay) resultOverlay->clearChildren();
}
