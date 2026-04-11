#include "Menu.hpp"
#include "ResourceManager.hpp"
#include "GeneratedLevels.hpp"
#include "utils/SettingsManager.hpp"
#include "utils/FileReader.hpp"
#include "utils/Logger.hpp"
#include "Colors.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <SFML/System/FileInputStream.hpp>

namespace fs = std::filesystem;


//  Конструктор

Menu::Menu(sf::RenderWindow& window, SettingsManager& settings) : 
    window(window), 
    settings(settings),
    titleLabel(ResourceManager::getFont("main"), "TOWER DEFENCE", 96),
    subTitleVersion(ResourceManager::getFont("main"), "v0.3a", 22),
    titleLevelSelect(ResourceManager::getFont("main"), "ВЫБОР УРОВНЯ", 64),
    titleSettings(ResourceManager::getFont("main"), "НАСТРОЙКИ", 64),
    lblWinScreen(ResourceManager::getFont("main"), "", 96)
{
    mainButtons.emplace_back(sf::Vector2f(0,0), sf::Vector2f(280, 58), "ИГРАТЬ");
    mainButtons.emplace_back(sf::Vector2f(0,0), sf::Vector2f(280, 58), "УЛУЧШЕНИЯ");
    mainButtons.emplace_back(sf::Vector2f(0,0), sf::Vector2f(280, 58), "НАСТРОЙКИ");
    mainButtons.emplace_back(sf::Vector2f(0,0), sf::Vector2f(280, 58), "ВЫХОД");

    updateViewSizes(window.getSize());
    scanLevels();
}


//  Обновление камер (инициализация, ресайз окна)

void Menu::updateViewSizes(sf::Vector2u windowSize) {
    float sw = static_cast<float>(windowSize.x);
    float sh = static_cast<float>(windowSize.y);
    float aspect = sw / sh;
    float uiH = sh / uiScale;
    float uiW = uiH * aspect;

    uiScale = settings.getFloat("ui_scale");

    uiView = sf::View(sf::FloatRect({ 0.f, 0.f }, { uiW, uiH }));

    sf::Vector2f ws = uiView.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;
    sf::Vector2f sz(280.f, 58.f);
    float gap = 16.f;

    // ЦИКЛ ПО КНОПКАМ: Расставляем их по вертикали
    for (int i = 0; i < (int)mainButtons.size(); i++) {
        float y = cy - 60.f + i * (sz.y + gap);
        mainButtons[i].setPosition({ cx - sz.x / 2.f, y });
    }
}


//  Сканирование уровней

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
    auto content = readFile(path);

    if (!content.has_value()) {
        LOGI("[ERROR]: Не удалось открыть файл карты: %s", path.c_str());
        return "Без названия";
    }

    std::istringstream f(content.value());
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t pos = line.find("name=");
        if (pos != std::string::npos) {
            std::string name = line.substr(pos + 5);
            if (!name.empty()) return name;
        }
        if (line.find("tiles=") != std::string::npos) break;
    }

    return "Без названия";
}


//  Вычисление геометрии — единственный источник истины о позициях

Menu::MainLayout Menu::computeMainLayout() const {
    MainLayout L;
    sf::Vector2f ws = uiView.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;
    sf::Vector2f sz(280.f, 58.f);
    constexpr float gap = 16.f;

    for (int i = 0; i < (int)mainButtons.size(); i++) {
        float y = cy - 60.f + i * (sz.y + gap);
        L.btns[i] = sf::FloatRect({ cx - sz.x / 2.f, y }, sz);
    }

    return L;
}

Menu::SettingsLayout Menu::computeSettingsLayout() const {
    SettingsLayout L;
    sf::Vector2f ws = uiView.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;

    float startY = cy - 220.f;
    float rowG = 90.f;  // Отступ между строками
    float controlX = cx - 50.f; // Элементы управления начинаются чуть левее центра
    float sliderW = 400.f;
    float btnW = 220.f;
    float btnH = 56.f;

    // Правый столбец (Элементы управления)
    L.musicSlider = sf::FloatRect({ controlX, startY }, { sliderW, 40.f });
    L.sfxSlider = sf::FloatRect({ controlX, startY + rowG }, { sliderW, 40.f });

    // Степперы (кнопки +/-)
    L.uiScaleMinus = sf::FloatRect({ controlX,           startY + rowG * 2 }, { 55.f, 55.f });
    L.uiScalePlus = sf::FloatRect({ controlX + 130.f,    startY + rowG * 2 }, { 55.f, 55.f });

    L.sensMinus = sf::FloatRect({ controlX,           startY + rowG * 3 }, { 55.f, 55.f });
    L.sensPlus = sf::FloatRect({ controlX + 130.f,    startY + rowG * 3 }, { 55.f, 55.f });

#ifndef ANDROID
    L.fullscreenToggle = sf::FloatRect({ controlX, startY + rowG * 4 }, { 300.f, 56.f });
#endif

    // Центрированные кнопки внизу
    float saveY = startY + rowG * 5.2f;
    float backY = startY + rowG * 6.2f;
    
    L.buttons.emplace_back(sf::Vector2f(cx - btnW / 2.f, saveY), sf::Vector2f(btnW, btnH), "СОХРАНИТЬ");
    L.buttons.emplace_back(sf::Vector2f(cx - btnW / 2.f, backY), sf::Vector2f(btnW, btnH), "НАЗАД");

    return L;
}

Menu::LevelSelectLayout Menu::computeLevelSelectLayout() const {
    LevelSelectLayout L;
    auto ws = uiView.getSize();
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

    L.buttons.emplace_back(sf::Vector2f(cx - btnW / 2.f, playY), sf::Vector2f(btnW, btnH), "ИГРАТЬ");
    L.buttons.emplace_back(sf::Vector2f(cx - btnW / 2.f, backY), sf::Vector2f(btnW, btnH), "НАЗАД");
    
    return L;
}

Menu::ResultOverlayLayout Menu::computeResultOverlayLayout() const {
    ResultOverlayLayout L;
    auto ws = uiView.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;

    const float btnW = 240.f, btnH = 58.f, btnGap = 16.f;
    float totalW = btnW * 2 + btnGap;
    
    L.buttons.emplace_back(sf::Vector2f(cx - totalW / 2.f, cy + 20.f), sf::Vector2f(btnW, btnH), "ИГРАТЬ СНОВА");
    L.buttons.emplace_back(sf::Vector2f(cx - totalW / 2.f + btnW + btnGap, cy + 20.f), sf::Vector2f(btnW, btnH), "ВЫБРАТЬ УРОВЕНЬ");
    
    return L;
}

//  Главное меню
void Menu::renderMain(const MainLayout& L) {
    auto& font = ResourceManager::getFont("main");
    auto ws = uiView.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;

    // Заголовок
    titleLabel.setCenteredX(ws.x / 2.f, cy - 260.f);
    titleLabel.render(window);

    // Версия
    subTitleVersion.setColor(sf::Color(120, 120, 120, 200));
    subTitleVersion.setCenteredX(cx, cy - 160.f);
    subTitleVersion.render(window);

    static const std::string labels[MainLayout::BTN_COUNT] = {
        "ИГРАТЬ", "УЛУЧШЕНИЯ", "НАСТРОЙКИ", "ВЫХОД"
    };
    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

    for (auto& btn : mainButtons) {
        btn.render(window, font, mouse);
    }
}

//  Выбор уровня
void Menu::renderLevelSelect(const LevelSelectLayout& L) {
    auto& font = ResourceManager::getFont("main");
    auto ws = uiView.getSize();
    float cx = ws.x / 2.f;
    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

    // Заголовок
    titleLevelSelect.setCenteredX(cx, 60.f);
    titleLevelSelect.render(window);

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
    
    // Кнопки "Играть" и "Назад"
    if (L.buttons.size() >= 2) {
        // Создаем копии, чтобы модифицировать (layout - const)
        auto playBtn = L.buttons[0];
        auto backBtn = L.buttons[1];
        playBtn.setEnabled(canPlay);
        playBtn.render(window, font, mouse);
        backBtn.render(window, font, mouse);
    }

    // Заметка о скролле (TODO)
    // При большом количестве уровней (> ~15-20) нужен вертикальный скролл.
    // Реализовать через scrollOffset (float), изменяемый колесом мыши или
    // свайпом. При рендере сдвигать gridStartY на -scrollOffset,
    // ограничивая диапазон [0, maxScroll]. Показывать скроллбар справа.
}

void Menu::renderSettings(const SettingsLayout& L) {
    auto& font = ResourceManager::getFont("main");
    auto ws = uiView.getSize();
    float cx = ws.x / 2.f;
    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

    // Заголовок
    titleSettings.setCenteredX(cx, 60.f);
    titleSettings.render(window);

    drawSlider("МУЗЫКА", L.musicSlider, settings.getFloat("music_volume"));
    drawSlider("ЗВУКИ", L.sfxSlider, settings.getFloat("sfx_volume"));

    drawStepper("МАСШТАБ UI", L.uiScaleMinus, L.uiScalePlus, std::to_string(settings.getFloat("ui_scale")).substr(0, 3));
    drawStepper("СЕНСА", L.sensMinus, L.sensPlus, std::to_string(settings.getFloat("sensitivity")).substr(0, 4));

#ifndef ANDROID
    // Рисуем лейбл для полноэкранного режима вручную
    std::string fsTxt = "ПОЛНЫЙ ЭКРАН";
    sf::Text fsLabel(font, sf::String::fromUtf8(fsTxt.begin(), fsTxt.end()), 24);
    fsLabel.setPosition({ L.fullscreenToggle.position.x - fsLabel.getLocalBounds().size.x - 40.f, L.fullscreenToggle.position.y + 10.f });
    window.draw(fsLabel);

    std::string stateText = settings.getBool("fullscreen") ? "ВКЛ" : "ВЫКЛ";
    drawBtn(stateText, L.fullscreenToggle, L.fullscreenToggle.contains(mouse));
#endif

    // Кнопки "Сохранить" и "Назад"
    if (L.buttons.size() >= 2) {
        L.buttons[0].render(window, font, mouse);
        L.buttons[1].render(window, font, mouse);
    }
}

void Menu::drawSlider(const std::string& label, sf::FloatRect r, float value) {
    auto& font = ResourceManager::getFont("main");
    // Текст
    sf::Text t(font, sf::String::fromUtf8(label.begin(), label.end()), 24);
    float labelX = r.position.x - t.getLocalBounds().size.x - 40.f;
    t.setPosition({ labelX, r.position.y + 2.f });
    window.draw(t);

    // Полоска
    sf::RectangleShape bar({ r.size.x, 8.f });
    bar.setPosition({ r.position.x, r.position.y + r.size.y / 2.f - 4.f });
    bar.setFillColor(sf::Color(60, 60, 60));
    window.draw(bar);

    // Прогресс
    sf::RectangleShape prog({ r.size.x * (value / 100.f), 8.f });
    prog.setPosition(bar.getPosition());
    prog.setFillColor(Colors::moneyText);
    window.draw(prog);

    // Ползунок
    sf::CircleShape cap(15.f);
    cap.setOrigin({ 15.f, 15.f });
    cap.setPosition({ r.position.x + r.size.x * (value / 100.f), r.position.y + r.size.y / 2.f });
    window.draw(cap);
}

void Menu::drawStepper(const std::string& label, sf::FloatRect rMinus, sf::FloatRect rPlus, std::string value) {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

    // 1. Рисуем подпись (слева от кнопок)
    sf::Text t(font, sf::String::fromUtf8(label.begin(), label.end()), 24);
    // Отступаем влево от кнопки "минус"
    float labelX = rMinus.position.x - t.getLocalBounds().size.x - 40.f;
    t.setPosition({ labelX, rMinus.position.y + 10.f });
    window.draw(t);

    drawBtn("-", rMinus, rMinus.contains(mouse));
    drawBtn("+", rPlus, rPlus.contains(mouse));

    // 3. Рисуем текущее значение между кнопками
    sf::Text valText(font, value, 24);
    valText.setFillColor(Colors::moneyText);
    valText.setPosition({ rPlus.position.x + rPlus.size.x + 20.f, rPlus.position.y + 10.f });

    // Считаем центр между кнопками
    float centerX = (rMinus.position.x + rMinus.size.x + rPlus.position.x) / 2.f;
    valText.setPosition({
        centerX - valText.getLocalBounds().size.x / 2.f,
        rMinus.position.y + 8.f
        });

    window.draw(valText);
}

void Menu::handleMainClick(sf::Vector2f pos, const MainLayout& L) {
    if (L.btns[Play].contains(pos))      state = MenuState::LevelSelect;
    else if (L.btns[Upgrades].contains(pos))  state = MenuState::Upgrades;
    else if (L.btns[Settings].contains(pos))  state = MenuState::Settings;
    else if (L.btns[Exit].contains(pos))      window.close();
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
    if (L.buttons.size() >= 1 && L.buttons[0].contains(pos) && !selectedLevel.empty()) {
        levelChosen = true;
        lastResult = SessionResult::None;
        return;
    }

    // Кнопка "Назад"
    if (L.buttons.size() >= 2 && L.buttons[1].contains(pos)) {
        state = MenuState::Main;
        selectedLevel = "";
        lastResult = SessionResult::None;
    }
}

void Menu::handleSettingsClick(sf::Vector2f pos, const SettingsLayout& L) {
    if (L.buttons.size() >= 2 && L.buttons[1].contains(pos)) {
        settings.save(); // Сохраняем при выходе из настроек
        state = MenuState::Main;
    }

    if (L.musicSlider.contains(pos)) {
        float val = (pos.x - L.musicSlider.position.x) / L.musicSlider.size.x * 100.f;
        settings.set("music_volume", std::clamp(val, 0.f, 100.f));
    }

    if (L.sfxSlider.contains(pos)) {
        float val = (pos.x - L.sfxSlider.position.x) / L.sfxSlider.size.x * 100.f;
        settings.set("sfx_volume", std::clamp(val, 0.f, 100.f));
    }

    // Кнопки масштаба
    if (L.uiScalePlus.contains(pos)) {
        float s = settings.getFloat("ui_scale") + 0.1f;
        settings.set("ui_scale", std::min(s, 3.0f));
        this->uiScale = s;
    }
    if (L.uiScaleMinus.contains(pos)) {
        float s = settings.getFloat("ui_scale") - 0.1f;
        settings.set("ui_scale", std::max(s, 0.5f));
        this->uiScale = s;
    }

    if (L.sensPlus.contains(pos)) {
        float s = settings.getFloat("sensitivity") + 0.1f;
        settings.set("sensitivity", std::min(s, 3.0f));
    }
    if (L.sensMinus.contains(pos)) {
        float s = settings.getFloat("sensitivity") - 0.1f;
        settings.set("sensitivity", std::max(s, 0.5f));
    }

    // Фуллскрин (только для ПК)
#ifndef ANDROID
    if (L.fullscreenToggle.contains(pos)) {
        bool current = settings.getBool("fullscreen");
        settings.set("fullscreen", !current);
        windowRecreationRequired = true;
    }
#endif

    if (L.buttons.size() >= 1 && L.buttons[0].contains(pos)) {
        settings.save();
        updateViewSizes(window.getSize());
        window.setView(uiView);
    }
}

//  Оверлей результата (поверх LevelSelect)
void Menu::renderResultOverlay() {
    auto& font = ResourceManager::getFont("main");
    auto ws = uiView.getSize();
    float cx = ws.x / 2.f;
    float cy = ws.y / 2.f;
    sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window), uiView);

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

    lblWinScreen.setText(header);
    lblWinScreen.setColor(headerColor);
    lblWinScreen.setCenteredX(cx, 180.f);
    lblWinScreen.render(window);

    // Подпись
    std::string sub = win ? "Уровень пройден!" : "База уничтожена...";
    sf::Text subText(font, sf::String::fromUtf8(sub.begin(), sub.end()), 28);
    subText.setFillColor(sf::Color(200, 200, 200, 220));
    subText.setPosition({ cx - subText.getLocalBounds().size.x / 2.f, cy - 70.f });
    window.draw(subText);

    // Кнопки оверлея
    auto overlayLayout = computeResultOverlayLayout();
    bool canPlayAgain = !lastLevelPath.empty();
    if (overlayLayout.buttons.size() >= 2) {
        overlayLayout.buttons[0].setEnabled(canPlayAgain);
        overlayLayout.buttons[0].render(window, font, mouse);
        overlayLayout.buttons[1].render(window, font, mouse);
    }
}

//  Обработка кликов по оверлею результата
//  (вызывается из handleEvents когда state == LevelSelect && lastResult != None)
void Menu::handleResultOverlayClick(sf::Vector2f pos, const ResultOverlayLayout& L) {
    if (L.buttons.size() < 2) return;
    
    // Кнопка "Играть снова"
    if (L.buttons[0].contains(pos) && !lastLevelPath.empty()) {
        selectedLevel = lastLevelPath;
        levelChosen = true;
        lastResult = SessionResult::None;
        return;
    }
    
    // Кнопка "Выбрать уровень"
    if (L.buttons[1].contains(pos)) {
        lastResult = SessionResult::None;
        selectedLevel = "";
        // просто закрываем оверлей — остаёмся в LevelSelect
    }
}

// Переопределяем handleEvents чтобы обработать клики по оверлею
void Menu::handleEvents() {
    auto mainLayout = computeMainLayout();
    auto levelLayout = computeLevelSelectLayout();
    auto settingsLayout = computeSettingsLayout();

    while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape) {
                if (lastResult != SessionResult::None) {
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
            sf::Vector2u newSize = { resized->size.x, resized->size.y };
            updateViewSizes(newSize);
            window.setView(uiView); // Устанавливаем обновленный вид в окно немедленно
            // Пересчитываем лейауты сразу после ресайза
            mainLayout = computeMainLayout();
            levelLayout = computeLevelSelectLayout();
            settingsLayout = computeSettingsLayout();
        }

        // --- ЕДИНАЯ ЛОГИКА ВВОДА ---
        sf::Vector2f pos(-1.f, -1.f);
        bool pressed = false;

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button == sf::Mouse::Button::Left) {
                pos = window.mapPixelToCoords(click->position, uiView); // ЯВНО uiView
                pressed = true;
            }
        }

        if (const auto* touch = event->getIf<sf::Event::TouchBegan>()) {
            pos = window.mapPixelToCoords(touch->position, uiView); // ЯВНО uiView
            pressed = true;
        }

        if (pressed && pos.x != -1.f) {
            // Оверлей результата
            if (state == MenuState::LevelSelect && lastResult != SessionResult::None) {
                auto overlayLayout = computeResultOverlayLayout();
                handleResultOverlayClick(pos, overlayLayout);
            }
            else {
                switch (state) {
                case MenuState::Main:
                    handleMainClick(pos, mainLayout);
                    break;
                case MenuState::LevelSelect:
                    handleLevelSelectClick(pos, levelLayout);
                    break;
                case MenuState::Settings:
                    handleSettingsClick(pos, settingsLayout);
                default:
                    break;
                }
            }
        }
        if (pressed) {
            mainLayout = computeMainLayout();
            levelLayout = computeLevelSelectLayout();
            auto settingsLayout = computeSettingsLayout(); // Считаем актуальные позиции
        }
    }
}

//  Заглушка
void Menu::renderStub(const std::string& title) {
    auto& font = ResourceManager::getFont("main");
    auto ws = uiView.getSize();
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
    window.setView(uiView);

    auto mainLayout = computeMainLayout();
    auto levelLayout = computeLevelSelectLayout();
    auto settingsLayout = computeSettingsLayout();

    switch (state) {
    case MenuState::Main:        renderMain(mainLayout);          break;
    case MenuState::LevelSelect:
        renderLevelSelect(levelLayout);
        if (lastResult != SessionResult::None) renderResultOverlay();
        break;
    case MenuState::Upgrades: renderStub("УЛУЧШЕНИЯ"); break;
    case MenuState::Settings: renderSettings(settingsLayout); break;
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