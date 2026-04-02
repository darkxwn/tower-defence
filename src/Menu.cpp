#include "Menu.hpp"
#include "Game.hpp"
#include "ResourceManager.hpp"
#include "GeneratedLevels.hpp"
#include "Colors.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cmath>

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────────
//  Конструктор
// ─────────────────────────────────────────────────────────────────────────────
Menu::Menu(sf::RenderWindow& window) : window(window) {
    scanLevels();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Сканирование уровней
// ─────────────────────────────────────────────────────────────────────────────
void Menu::scanLevels() {
    levels.clear();
#ifdef ANDROID
    std::string dir = "levels/";
#else
    std::string dir = "data/levels/";
#endif
    std::vector<std::string> mapNames = getLevelList();

    for (int i = 0; i < (int)mapNames.size(); ++i) {
        std::string fullPath = dir + mapNames[i];
        levels.push_back({ fullPath, readLevelName(fullPath), i });
    }
}

std::string Menu::readLevelName(const std::string& path) const {
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind("name=", 0) == 0) return line.substr(5);
        if (line == "tiles=") break;
    }
    return "Уровень";
}

// ─────────────────────────────────────────────────────────────────────────────
//  Вычисление геометрии — единственный источник истины о позициях
// ─────────────────────────────────────────────────────────────────────────────
Menu::MainLayout Menu::computeMainLayout() const {
    MainLayout L;
    auto ws = window.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;
    sf::Vector2f sz(280.f, 58.f);
    constexpr float gap = 16.f;

    for (int i = 0; i < MainLayout::BTN_COUNT; i++) {
        float y = cy - 60.f + i * (sz.y + gap);
        L.btns[i] = sf::FloatRect({ cx - sz.x / 2.f, y }, sz);
    }
    return L;
}

Menu::LevelSelectLayout Menu::computeLevelSelectLayout() const {
    LevelSelectLayout L;
    auto ws = window.getSize();
    float cx = ws.x / 2.f;

    const float cardW = 200.f;
    const float cardH = 120.f;
    const float cardGap = 24.f;
    const float marginX = 80.f;  // отступ от края экрана слева и справа

    // Сколько карточек влезает в строку при текущей ширине окна
    float availW = ws.x - 2.f * marginX;
    L.rowSize = std::max(1, (int)((availW + cardGap) / (cardW + cardGap)));
    L.rowCount = ((int)levels.size() + L.rowSize - 1) / L.rowSize;

    float gridW = L.rowSize * cardW + (L.rowSize - 1) * cardGap;
    float startX = cx - gridW / 2.f;
    // Сетка карточек начинается под заголовком
    float gridStartY = 160.f;

    for (int i = 0; i < (int)levels.size(); i++) {
        int row = i / L.rowSize;
        int col = i % L.rowSize;
        float x = startX + col * (cardW + cardGap);
        float y = gridStartY + row * (cardH + cardGap);
        L.cards.push_back({ sf::FloatRect({x, y}, {cardW, cardH}), i });
    }

    // Кнопки под сеткой
    float btnW = 220.f;
    float btnH = 56.f;
    float btnGap = 14.f;
    float gridBot = gridStartY + L.rowCount * (cardH + cardGap) - cardGap;
    float playY = gridBot + 36.f;
    float backY = playY + btnH + btnGap;

    L.playBtn = sf::FloatRect({ cx - btnW / 2.f, playY }, { btnW, btnH });
    L.backBtn = sf::FloatRect({ cx - btnW / 2.f, backY }, { btnW, btnH });
    return L;
}

//  Главное меню
void Menu::renderMain(const MainLayout& L) {
    auto& font = ResourceManager::getFont("main");
    auto ws = window.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;

    // Заголовок
    std::string title = "TOWER DEFENCE";
    sf::Text titleText(font, sf::String::fromUtf8(title.begin(), title.end()), 96);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition({ cx - titleText.getLocalBounds().size.x / 2.f, cy - 260.f });
    window.draw(titleText);

    // Версия
    std::string ver = "v0.1";
    sf::Text verText(font, sf::String::fromUtf8(ver.begin(), ver.end()), 22);
    verText.setFillColor(sf::Color(120, 120, 120, 200));
    verText.setPosition({ cx - verText.getLocalBounds().size.x / 2.f, cy - 160.f });
    window.draw(verText);

    static const std::string labels[MainLayout::BTN_COUNT] = {
        "ИГРАТЬ", "УЛУЧШЕНИЯ", "НАСТРОЙКИ", "ВЫХОД"
    };
    sf::Vector2f mouse = sf::Vector2f(sf::Mouse::getPosition(window));

    for (int i = 0; i < MainLayout::BTN_COUNT; i++)
        drawBtn(labels[i], L.btns[i], L.btns[i].contains(mouse));
}

void Menu::handleMainClick(sf::Vector2f pos, const MainLayout& L) {
    for (int i = 0; i < MainLayout::BTN_COUNT; i++) {
        if (!L.btns[i].contains(pos)) continue;
        switch (i) {
        case 0: state = MenuState::LevelSelect; break;
        case 1: state = MenuState::Upgrades;    break;
        case 2: state = MenuState::Settings;    break;
        case 3: window.close();                 break;
        }
        return;
    }
}

//  Выбор уровня
void Menu::renderLevelSelect(const LevelSelectLayout& L) {
    auto& font = ResourceManager::getFont("main");
    auto ws = window.getSize();
    float cx = ws.x / 2.f;
    sf::Vector2f mouse = sf::Vector2f(sf::Mouse::getPosition(window));

    // Заголовок
    std::string title = "ВЫБОР УРОВНЯ";
    sf::Text titleText(font, sf::String::fromUtf8(title.begin(), title.end()), 64);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition({ cx - titleText.getLocalBounds().size.x / 2.f, 60.f });
    window.draw(titleText);

    // Карточки уровней
    for (const auto& card : L.cards) {
        const LevelInfo& lvl = levels[card.levelIndex];
        bool hovered = card.bounds.contains(mouse);
        bool selected = (lvl.filePath == selectedLevel);

        sf::RectangleShape bg(card.bounds.size);
        bg.setPosition(card.bounds.position);
        bg.setFillColor(selected ? sf::Color(55, 55, 55, 230)
            : hovered ? sf::Color(48, 48, 48, 220)
            : Colors::slotBg);
        if (selected) {
            bg.setOutlineColor(Colors::moneyText);
            bg.setOutlineThickness(2.f);
        }
        window.draw(bg);

        // "Уровень N"
        std::string numStr = "УРОВЕНЬ " + std::to_string(lvl.index + 1);
        sf::Text numText(font, sf::String::fromUtf8(numStr.begin(), numStr.end()), 18);
        numText.setFillColor(sf::Color(150, 150, 150, 255));
        numText.setPosition({
            card.bounds.position.x + (card.bounds.size.x - numText.getLocalBounds().size.x) / 2.f,
            card.bounds.position.y + 16.f
            });
        window.draw(numText);

        // Название уровня
        sf::Text nameText(font, sf::String::fromUtf8(lvl.name.begin(), lvl.name.end()), 20);
        nameText.setFillColor(sf::Color::White);
        nameText.setPosition({
            card.bounds.position.x + (card.bounds.size.x - nameText.getLocalBounds().size.x) / 2.f,
            card.bounds.position.y + 50.f
            });
        window.draw(nameText);
    }

    // Кнопки
    bool canPlay = !selectedLevel.empty();
    drawBtn("ИГРАТЬ", L.playBtn, canPlay && L.playBtn.contains(mouse), canPlay);
    drawBtn("НАЗАД", L.backBtn, L.backBtn.contains(mouse));

    // Заметка о скролле (TODO)
    // При большом количестве уровней (> ~15-20) нужен вертикальный скролл.
    // Реализовать через scrollOffset (float), изменяемый колесом мыши или
    // свайпом. При рендере сдвигать gridStartY на -scrollOffset,
    // ограничивая диапазон [0, maxScroll]. Показывать скроллбар справа.
}

void Menu::handleLevelSelectClick(sf::Vector2f pos, const LevelSelectLayout& L) {
    // Клик по карточке уровня
    for (const auto& card : L.cards) {
        if (!card.bounds.contains(pos)) continue;
        const std::string& fp = levels[card.levelIndex].filePath;
        // Повторный клик по выбранному снимает выбор
        selectedLevel = (selectedLevel == fp) ? "" : fp;
        lastResult = SessionResult::None; // снимаем баннер при смене выбора
        return;
    }

    // Кнопка "Играть"
    if (L.playBtn.contains(pos) && !selectedLevel.empty()) {
        levelChosen = true;
        lastResult = SessionResult::None;
        return;
    }

    // Кнопка "Назад"
    if (L.backBtn.contains(pos)) {
        state = MenuState::Main;
        selectedLevel = "";
        lastResult = SessionResult::None;
    }
}

//  Оверлей результата (поверх LevelSelect)
void Menu::renderResultOverlay() {
    auto& font = ResourceManager::getFont("main");
    auto ws = window.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;
    sf::Vector2f mouse = sf::Vector2f(sf::Mouse::getPosition(window));

    // Полупрозрачное затемнение
    sf::RectangleShape overlayRect;
    overlayRect.setSize(sf::Vector2f(ws));
    overlayRect.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlayRect);

    // Заголовок результата
    bool win = (lastResult == SessionResult::Win);
    std::string header = win ? "ПОБЕДА!" : "ПОРАЖЕНИЕ";
    sf::Color   headerColor = win ? sf::Color(255, 200, 37, 255)
        : sf::Color(234, 50, 60, 255);
    sf::Text headerText(font, sf::String::fromUtf8(header.begin(), header.end()), 96);
    headerText.setFillColor(headerColor);
    headerText.setPosition({ cx - headerText.getLocalBounds().size.x / 2.f, cy - 180.f });
    window.draw(headerText);

    // Подпись
    std::string sub = win ? "Уровень пройден!" : "База уничтожена...";
    sf::Text subText(font, sf::String::fromUtf8(sub.begin(), sub.end()), 28);
    subText.setFillColor(sf::Color(200, 200, 200, 220));
    subText.setPosition({ cx - subText.getLocalBounds().size.x / 2.f, cy - 70.f });
    window.draw(subText);

    // Кнопки: "Играть снова" и "Выбрать уровень"
    const float btnW = 240.f, btnH = 58.f, btnGap = 16.f;
    float totalW = btnW * 2 + btnGap;
    sf::FloatRect againBtn({ cx - totalW / 2.f, cy + 20.f }, { btnW, btnH });
    sf::FloatRect selectBtn({ cx - totalW / 2.f + btnW + btnGap, cy + 20.f }, { btnW, btnH });

    drawBtn("ИГРАТЬ СНОВА", againBtn, againBtn.contains(mouse), !lastLevelPath.empty());
    drawBtn("ВЫБРАТЬ УРОВЕНЬ", selectBtn, selectBtn.contains(mouse));

    // Обработка кликов прямо здесь — оверлей активен только поверх LevelSelect
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        // Клики обрабатываются в handleLevelSelectClick через обычный поток событий.
        // Кнопки оверлея обрабатываем отдельно через флаги — см. handleEvents.
        // (Этот метод только рисует; клики — в handleEvents через overlayClick)
    }
}

//  Обработка кликов по оверлею результата
//  (вызывается из handleEvents когда state == LevelSelect && lastResult != None)
static void handleResultOverlayClick(sf::Vector2f pos,
    sf::Vector2u ws, SessionResult& lastResult,
    std::string& lastLevelPath, std::string& selectedLevel,
    bool& levelChosen) {
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;
    const float btnW = 240.f, btnH = 58.f, btnGap = 16.f;
    float totalW = btnW * 2 + btnGap;

    sf::FloatRect againBtn({ cx - totalW / 2.f,              cy + 20.f }, { btnW, btnH });
    sf::FloatRect selectBtn({ cx - totalW / 2.f + btnW + btnGap, cy + 20.f }, { btnW, btnH });

    if (againBtn.contains(pos) && !lastLevelPath.empty()) {
        selectedLevel = lastLevelPath;
        levelChosen = true;
        lastResult = SessionResult::None;
        return;
    }
    if (selectBtn.contains(pos)) {
        lastResult = SessionResult::None;
        selectedLevel = "";
        // просто закрываем оверлей — остаёмся в LevelSelect
    }
}

// Переопределяем handleEvents чтобы обработать клики по оверлею
void Menu::handleEvents() {
    auto mainL = computeMainLayout();
    auto levelL = computeLevelSelectLayout();

    while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                if (lastResult != SessionResult::None) {
                    // Escape закрывает оверлей результата
                    lastResult = SessionResult::None;
                    selectedLevel = "";
                }
                else if (state != MenuState::Main) {
                    state = MenuState::Main;
                }
                else {
                    window.close();
                }
            }
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::FloatRect view({ 0.f, 0.f },
                sf::Vector2f(sf::Vector2u{ resized->size.x, resized->size.y }));
            Game::updateViewSizes(view.size)
            mainL = computeMainLayout();
            levelL = computeLevelSelectLayout();
        }

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button != sf::Mouse::Button::Left) continue;
            sf::Vector2f pos = sf::Vector2f(click->position);

            // Оверлей результата перехватывает клики первым
            if (state == MenuState::LevelSelect && lastResult != SessionResult::None) {
                handleResultOverlayClick(pos, window.getSize(),
                    lastResult, lastLevelPath, selectedLevel, levelChosen);
                continue;
            }

            switch (state) {
            case MenuState::Main:
                handleMainClick(pos, mainL);
                break;
            case MenuState::LevelSelect:
                handleLevelSelectClick(pos, levelL);
                break;
            default:
                break;
            }
        }

        if (const auto* touch = event->getIf<sf::Event::TouchBegan>()) {
            sf::Vector2f pos = sf::Vector2f(touch->position);
            if (pos.x != -1.f) {
                switch (state) {
                case MenuState::Main:
                    handleMainClick(pos, mainL);
                    break;
                case MenuState::LevelSelect:
                    handleLevelSelectClick(pos, levelL);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

//  Заглушка
void Menu::renderStub(const std::string& title) {
    auto& font = ResourceManager::getFont("main");
    auto ws = window.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;

    sf::Text t(font, sf::String::fromUtf8(title.begin(), title.end()), 72);
    t.setFillColor(sf::Color(100, 100, 100, 200));
    t.setPosition({ cx - t.getLocalBounds().size.x / 2.f, cy - 100.f });
    window.draw(t);

    std::string sub = "В РАЗРАБОТКЕ";
    sf::Text st(font, sf::String::fromUtf8(sub.begin(), sub.end()), 28);
    st.setFillColor(sf::Color(70, 70, 70, 200));
    st.setPosition({ cx - st.getLocalBounds().size.x / 2.f, cy + 10.f });
    window.draw(st);
}

//  Отрисовка кнопки
void Menu::drawBtn(const std::string& label, sf::FloatRect r,
    bool hovered, bool enabled, sf::Color customFill) const {
    auto& font = ResourceManager::getFont("main");

    sf::RectangleShape bg(r.size);
    bg.setPosition(r.position);

    if (customFill.a > 0) {
        bg.setFillColor(customFill);
    }
    else if (!enabled) {
        bg.setFillColor(sf::Color(35, 35, 35, 140));
    }
    else if (hovered) {
        bg.setFillColor(sf::Color(72, 72, 72, 230));
    }
    else {
        bg.setFillColor(Colors::slotBg);
    }
    window.draw(bg);

    sf::Text text(font, sf::String::fromUtf8(label.begin(), label.end()), 24);
    text.setFillColor(enabled ? sf::Color::White : sf::Color(80, 80, 80, 180));
    text.setPosition({
        r.position.x + (r.size.x - text.getLocalBounds().size.x) / 2.f,
        r.position.y + (r.size.y - text.getLocalBounds().size.y) / 2.f - 4.f
        });
    window.draw(text);
}

//  Публичный интерфейс
void Menu::render() {
    window.clear(Colors::gameBg);

    auto mainL = computeMainLayout();
    auto levelL = computeLevelSelectLayout();

    switch (state) {
    case MenuState::Main:        renderMain(mainL);          break;
    case MenuState::LevelSelect:
        renderLevelSelect(levelL);
        if (lastResult != SessionResult::None) renderResultOverlay();
        break;
    case MenuState::Upgrades: renderStub("УЛУЧШЕНИЯ"); break;
    case MenuState::Settings: renderStub("НАСТРОЙКИ"); break;
    }

    window.display();
}

bool        Menu::isLevelChosen()  const { return levelChosen; }
std::string Menu::getChosenLevel() const { return selectedLevel; }

void Menu::resetChoice() {
    // Не сбрасываем selectedLevel — игрок вернётся в LevelSelect
    // с уже выбранным уровнем, что удобно для быстрого перезапуска
    levelChosen = false;
    state = MenuState::LevelSelect;
}

void Menu::notifyResult(SessionResult result, const std::string& levelPath) {
    lastResult = result;
    lastLevelPath = levelPath;
    // Переходим в LevelSelect чтобы показать оверлей результата
    state = MenuState::LevelSelect;
}