#include "Menu.hpp"
#include "ResourceManager.hpp"
#include "Colors.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

Menu::Menu(sf::RenderWindow& window) : window(window) {
    scanLevels();
}

// Сканирует папку data/levels/ и собирает информацию об уровнях
void Menu::scanLevels() {
    levels.clear();
    const std::string levelsDir = "data/levels/";

    if (!fs::exists(levelsDir)) {
        std::cerr << "[Меню]: Папка уровней не найдена: " << levelsDir << "\n";
        return;
    }

    // Собираем все .map файлы и сортируем по имени файла
    std::vector<fs::path> mapFiles;
    for (const auto& entry : fs::directory_iterator(levelsDir)) {
        if (entry.path().extension() == ".map")
            mapFiles.push_back(entry.path());
    }
    std::sort(mapFiles.begin(), mapFiles.end());

    for (int i = 0; i < (int)mapFiles.size(); i++) {
        LevelInfo info;
        info.filePath = mapFiles[i].string();
        info.index    = i;
        info.name     = readLevelName(info.filePath);
        levels.push_back(info);
    }
}

// Читает поле name= из .map файла; если не найдено — возвращает "Уровень N"
std::string Menu::readLevelName(const std::string& path) const {
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("name=", 0) == 0)
            return line.substr(5);
        if (line == "tiles=") break; // дальше тайлы — имя не найдено
    }
    return "Уровень";
}

// Главный цикл меню
void Menu::handleEvents() {
    while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // Escape из подменю возвращает в главное
            if (key->code == sf::Keyboard::Key::Escape) {
                if (state != MenuState::Main)
                    state = MenuState::Main;
                else
                    window.close();
            }
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::FloatRect view({ 0.f, 0.f }, sf::Vector2f(sf::Vector2u{ resized->size.x, resized->size.y }));
            window.setView(sf::View(view));
        }

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button != sf::Mouse::Button::Left) continue;
            sf::Vector2f pos = sf::Vector2f(click->position);

            switch (state) {
                case MenuState::Main:        handleMainClick(pos);        break;
                case MenuState::LevelSelect: handleLevelSelectClick(pos); break;
                default: break; // Settings/Upgrades — только Escape
            }
        }
    }
}

void Menu::render() {
    window.clear(Colors::gameBg);

    switch (state) {
        case MenuState::Main:        renderMain();              break;
        case MenuState::LevelSelect: renderLevelSelect();       break;
        case MenuState::Upgrades:    renderStub("УЛУЧШЕНИЯ");   break;
        case MenuState::Settings:    renderStub("НАСТРОЙКИ");   break;
    }

    window.display();
}

// Отрисовка главного меню 
void Menu::renderMain() {
    auto& font = ResourceManager::getFont("main");
    auto winSize = window.getSize();
    float cx = winSize.x / 2.f;
    float cy = winSize.y / 2.f;

    // Заголовок игры
    std::string title = "TOWER DEFENCE";
    sf::Text titleText(font, sf::String::fromUtf8(title.begin(), title.end()), 96);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition({ cx - titleText.getLocalBounds().size.x / 2.f, cy - 260.f });
    window.draw(titleText);

    // Подзаголовок
    std::string sub = "v0.1";
    sf::Text subText(font, sf::String::fromUtf8(sub.begin(), sub.end()), 22);
    subText.setFillColor(sf::Color(120, 120, 120, 200));
    subText.setPosition({ cx - subText.getLocalBounds().size.x / 2.f, cy - 160.f });
    window.draw(subText);

    // Кнопки главного меню
    const float btnW = 280.f, btnH = 58.f, btnGap = 16.f;
    float btnX = cx - btnW / 2.f;

    struct Item { std::string label; };
    std::vector<Item> items = {
        { "ИГРАТЬ" },
        { "УЛУЧШЕНИЯ" },
        { "НАСТРОЙКИ" },
        { "ВЫХОД" }
    };

    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));

    for (int i = 0; i < (int)items.size(); i++) {
        float btnY = cy - 60.f + i * (btnH + btnGap);
        sf::FloatRect bounds(sf::Vector2f(btnX, btnY), sf::Vector2f(btnW, btnH));
        bool hovered = bounds.contains(mousePos);
        drawButton(items[i].label, { btnX, btnY }, { btnW, btnH }, hovered);
    }
}

// Обработка кликов в главном меню 
void Menu::handleMainClick(sf::Vector2f pos) {
    auto winSize = window.getSize();
    float cx = winSize.x / 2.f;
    float cy = winSize.y / 2.f;

    const float btnW = 280.f, btnH = 58.f, btnGap = 16.f;
    float btnX = cx - btnW / 2.f;

    // Индексы кнопок: 0=Играть, 1=Улучшения, 2=Настройки, 3=Выход
    for (int i = 0; i < 4; i++) {
        float btnY = cy - 60.f + i * (btnH + btnGap);
        sf::FloatRect bounds(sf::Vector2f(btnX, btnY), sf::Vector2f(btnW, btnH));
        if (!bounds.contains(pos)) continue;

        switch (i) {
            case 0: state = MenuState::LevelSelect; break;
            case 1: state = MenuState::Upgrades;    break;
            case 2: state = MenuState::Settings;    break;
            case 3: window.close();                 break;
        }
        return;
    }
}

// Отрисовка выбора уровня
void Menu::renderLevelSelect() {
    auto& font = ResourceManager::getFont("main");
    auto winSize = window.getSize();
    float cx = winSize.x / 2.f;

    // Заголовок
    std::string title = "ВЫБОР УРОВНЯ";
    sf::Text titleText(font, sf::String::fromUtf8(title.begin(), title.end()), 64);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition({ cx - titleText.getLocalBounds().size.x / 2.f, 60.f });
    window.draw(titleText);

    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));

    // Плашки уровней — расположены в ряд по центру
    const float cardW = 200.f, cardH = 120.f, cardGap = 24.f;
    int count = (int)levels.size();
    float totalW = count * cardW + (count - 1) * cardGap;
    float startX = cx - totalW / 2.f;
    float cardY = winSize.y / 2.f - cardH / 2.f - 30.f;

    for (int i = 0; i < count; i++) {
        float cardX = startX + i * (cardW + cardGap);
        sf::FloatRect bounds(sf::Vector2f(cardX, cardY), sf::Vector2f(cardW, cardH));
        bool hovered = bounds.contains(mousePos);
        bool selected = (levels[i].filePath == selectedLevel);

        // Фон карточки
        sf::RectangleShape card(sf::Vector2f(cardW, cardH));
        card.setPosition({ cardX, cardY });
        card.setFillColor(selected ? sf::Color(50, 50, 50, 230)
                        : hovered  ? sf::Color(45, 45, 45, 210)
                                   : Colors::slotBg);
        if (selected) {
            card.setOutlineColor(Colors::moneyText);
            card.setOutlineThickness(2.f);
        }
        window.draw(card);

        // "Уровень N"
        std::string numStr = "УРОВЕНЬ " + std::to_string(levels[i].index + 1);
        sf::Text numText(font, sf::String::fromUtf8(numStr.begin(), numStr.end()), 20);
        numText.setFillColor(sf::Color(160, 160, 160, 255));
        numText.setPosition({ cardX + (cardW - numText.getLocalBounds().size.x) / 2.f,
                              cardY + 18.f });
        window.draw(numText);

        // Название уровня
        auto name = levels[i].name;
        sf::Text nameText(font, sf::String::fromUtf8(name.begin(), name.end()), 22);
        nameText.setFillColor(sf::Color::White);
        nameText.setPosition({ cardX + (cardW - nameText.getLocalBounds().size.x) / 2.f,
                               cardY + 52.f });
        window.draw(nameText);
    }

    // Кнопка "Играть" — активна только если уровень выбран
    const float btnW = 220.f, btnH = 56.f;
    float btnX = cx - btnW / 2.f;
    float btnY = cardY + cardH + 50.f;
    bool canPlay = !selectedLevel.empty();
    sf::FloatRect playBounds(sf::Vector2f(btnX, btnY), sf::Vector2f(btnW, btnH));
    bool playHovered = canPlay && playBounds.contains(mousePos);
    drawButton("ИГРАТЬ", { btnX, btnY }, { btnW, btnH }, playHovered, canPlay);

    // Кнопка "Назад"
    float backBtnX = cx - btnW / 2.f;
    float backBtnY = btnY + btnH + 16.f;
    sf::FloatRect backBounds(sf::Vector2f(backBtnX, backBtnY), sf::Vector2f(btnW, btnH));
    bool backHovered = backBounds.contains(mousePos);
    drawButton("НАЗАД", { backBtnX, backBtnY }, { btnW, btnH }, backHovered);
}

// Обработка кликов в выборе уровня
void Menu::handleLevelSelectClick(sf::Vector2f pos) {
    auto winSize = window.getSize();
    float cx = winSize.x / 2.f;

    const float cardW = 200.f, cardH = 120.f, cardGap = 24.f;
    int count = (int)levels.size();
    float totalW = count * cardW + (count - 1) * cardGap;
    float startX = cx - totalW / 2.f;
    float cardY = winSize.y / 2.f - cardH / 2.f - 30.f;

    // Клик по карточке уровня
    for (int i = 0; i < count; i++) {
        float cardX = startX + i * (cardW + cardGap);
        sf::FloatRect bounds(sf::Vector2f(cardX, cardY), sf::Vector2f(cardW, cardH));
        if (bounds.contains(pos)) {
            // Повторный клик по уже выбранному — снимает выбор
            selectedLevel = (selectedLevel == levels[i].filePath) ? "" : levels[i].filePath;
            return;
        }
    }

    // Кнопка "Играть"
    const float btnW = 220.f, btnH = 56.f;
    float btnX = cx - btnW / 2.f;
    float btnY = cardY + cardH + 50.f;
    sf::FloatRect playBounds(sf::Vector2f(btnX, btnY), sf::Vector2f(btnW, btnH));
    if (playBounds.contains(pos) && !selectedLevel.empty()) {
        levelChosen = true;
        return;
    }

    // Кнопка "Назад"
    float backBtnY = btnY + btnH + 16.f;
    sf::FloatRect backBounds(sf::Vector2f(btnX, backBtnY), sf::Vector2f(btnW, btnH));
    if (backBounds.contains(pos)) {
        state = MenuState::Main;
        selectedLevel = "";
    }
}

// Заглушка для Settings / Upgrades
void Menu::renderStub(const std::string& title) {
    auto& font = ResourceManager::getFont("main");
    auto winSize = window.getSize();
    float cx = winSize.x / 2.f;
    float cy = winSize.y / 2.f;

    sf::Text titleText(font, sf::String::fromUtf8(title.begin(), title.end()), 72);
    titleText.setFillColor(sf::Color(100, 100, 100, 200));
    titleText.setPosition({ cx - titleText.getLocalBounds().size.x / 2.f, cy - 120.f });
    window.draw(titleText);

    std::string sub = "В РАЗРАБОТКЕ";
    sf::Text subText(font, sf::String::fromUtf8(sub.begin(), sub.end()), 28);
    subText.setFillColor(sf::Color(70, 70, 70, 200));
    subText.setPosition({ cx - subText.getLocalBounds().size.x / 2.f, cy });
    window.draw(subText);

    // Кнопка "Назад"
    const float btnW = 220.f, btnH = 56.f;
    float btnX = cx - btnW / 2.f;
    float btnY = cy + 80.f;
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
    sf::FloatRect bounds(sf::Vector2f(btnX, btnY), sf::Vector2f(btnW, btnH));

    // Обрабатываем клик прямо из render (нет смысла дублировать для заглушки)
    bool hovered = bounds.contains(mousePos);
    drawButton("НАЗАД", { btnX, btnY }, { btnW, btnH }, hovered);

    // Клик обрабатывается через handleEvents -> default case не перехватывает,
    // поэтому для заглушек достаточно Escape — см. handleEvents
}

// Вспомогательный метод отрисовки кнопки
sf::FloatRect Menu::drawButton(const std::string& label, sf::Vector2f pos, sf::Vector2f size,
                                bool hovered, bool enabled) {
    auto& font = ResourceManager::getFont("main");

    sf::RectangleShape btn(size);
    btn.setPosition(pos);
    if (!enabled)
        btn.setFillColor(sf::Color(35, 35, 35, 160));
    else if (hovered)
        btn.setFillColor(sf::Color(70, 70, 70, 230));
    else
        btn.setFillColor(Colors::slotBg);
    window.draw(btn);

    sf::Text text(font, sf::String::fromUtf8(label.begin(), label.end()), 24);
    text.setFillColor(enabled ? sf::Color::White : sf::Color(80, 80, 80, 200));
    text.setPosition({ pos.x + (size.x - text.getLocalBounds().size.x) / 2.f,
                       pos.y + (size.y - text.getLocalBounds().size.y) / 2.f - 4.f });
    window.draw(text);

    return sf::FloatRect(pos, size);
}

// Геттеры
bool Menu::isLevelChosen() const  { return levelChosen; }
std::string Menu::getChosenLevel() const { return selectedLevel; }
void Menu::resetChoice()           { levelChosen = false; selectedLevel = ""; state = MenuState::Main; }
