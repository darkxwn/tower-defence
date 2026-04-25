#include "HUD.hpp"
#include "ResourceManager.hpp"
#include "GameData.hpp"
#include "Colors.hpp"
#include <SFML/Graphics.hpp>

// Конструктор инициализирует кнопки и их логику
HUD::HUD() {
    // кэширование основных ресурсов
    mainFont = &ResourceManager::getFont("main");
    coinTex = &ResourceManager::get("icon-coins");
    heartTex = &ResourceManager::get("icon-heart");

    // кэширование текстур скорости
    for (int i = 0; i < 3; ++i) {
        speedTexs[i] = &ResourceManager::get("icon-speed" + std::to_string(i + 1));
    }

    // Инициализация управляющих кнопок
    pauseBtn = UI::Button(ResourceManager::get("icon-pause"), sf::Vector2f(64.f, 64.f));
    pauseBtn.setIconScale({ 0.666f, 0.666f });
    pauseBtn.setTransparent(true);
    pauseBtn.setCallback([this]() { pauseRequested = true; });

    skipBtn = UI::Button(ResourceManager::get("icon-start"), sf::Vector2f(48.f, 48.f));
    skipBtn.setIconScale({ 0.5f, 0.5f });
    skipBtn.setTransparent(true);
    skipBtn.setCallback([this]() { skipRequested = true; });

    speedBtn = UI::Button(*speedTexs[0], sf::Vector2f(96.f, 96.f));
    speedBtn.setTransparent(true);
    speedBtn.setCallback([this]() {
        speedMode = (speedMode + 1) % 3;
        speedBtn.setTexture(*speedTexs[speedMode]); // использование кэшированного указателя
    });

    // Инициализация кнопок управления башней
    upgradeBtn = UI::Button(ResourceManager::get("icon-upgrade"), sf::Vector2f(36.f, 36.f));
    upgradeBtn.setIconScale({ 0.375f, 0.375f }); 
    upgradeBtn.setCallback([this]() { 
        float elapsed = doubleClickClock.getElapsedTime().asSeconds();
        if (lastClickedBtnId == 1 && elapsed < doubleClickTime) {
            upgradeRequested = true;
            lastClickedBtnId = 0;
            upgradeBtn.setIconColor(sf::Color::White);
        } else {
            lastClickedBtnId = 1;
            doubleClickClock.restart();
            upgradeBtn.setIconColor(sf::Color::Yellow);
            sellBtn.setIconColor(sf::Color::White);
        }
    });

    sellBtn = UI::Button(ResourceManager::get("icon-sell"), sf::Vector2f(36.f, 36.f));
    sellBtn.setIconScale({ 0.375f, 0.375f });
    sellBtn.setCallback([this]() { 
        float elapsed = doubleClickClock.getElapsedTime().asSeconds();
        if (lastClickedBtnId == 2 && elapsed < doubleClickTime) {
            sellRequested = true;
            lastClickedBtnId = 0;
            sellBtn.setIconColor(sf::Color::White);
        } else {
            lastClickedBtnId = 2;
            doubleClickClock.restart();
            sellBtn.setIconColor(sf::Color(255, 100, 100));
            upgradeBtn.setIconColor(sf::Color::White);
        }
    });

    // Создание слотов для башен
    auto towerNames = GameData::getTowerNames();
    towerSlots.reserve((int)towerNames.size());
    for (int i = 0; i < (int)towerNames.size(); i++) {
        int cost = GameData::getTower(towerNames[i]).cost;
        UI::Button slot(ResourceManager::get("tower-" + towerNames[i] + "-preview"), *mainFont, std::to_string(cost) + "$", sf::Vector2f(90.f, 100.f), UI::IconPlacement::Top);
        slot.setIconScale(sf::Vector2f(0.15625f, 0.15625f));
        slot.setTextSize(16);
        slot.setTextColor(Colors::Theme::TextMoney);
        slot.setCallback([this, i]() {
            selectedTowerSlot = (selectedTowerSlot == i) ? -1 : i;
        });
        towerSlots.push_back(std::move(slot));
    }
}

// Расставляет элементы интерфейса согласно размеру логического экрана
void HUD::updateLayout(sf::Vector2f viewSize, float uiScale) {
    this->uiScale = uiScale;
    float cx = viewSize.x / 2.f;

    pauseBtn.setPosition({ 20.f, 20.f });
    skipBtn.setPosition({ cx + 130.f, 30.f });
    speedBtn.setPosition({ 15.0f, viewSize.y - 111.0f });

    float panelWidth = (2 + (int)towerSlots.size()) * 100.f;
    float startXPos = cx - panelWidth / 2.f;
    float slotY = viewSize.y - 110.f;

    for (int i = 0; i < (int)towerSlots.size(); i++) {
        towerSlots[i].setPosition({ (startXPos + 100.f) + i * 100.f, slotY });
    }
}

void HUD::setUiScale(float scale) {
    uiScale = scale;
}

// Отрисовывает интерфейс: панели, ресурсы, волны и кнопки
void HUD::render(sf::RenderWindow& window, int money, int lives, int wave, WaveState state) {
    if (!mainFont || !coinTex || !heartTex) return; // проверка валидности ресурсов

    sf::Vector2f ws = window.getView().getSize();
    float cx = ws.x / 2.f;
    float s = uiScale;

    pauseBtn.render(window);

    // Верхняя панель волн
    float topPanelHeight = 85.f;
    float topWidth = std::max(300.f * s, 250.f);
    sf::ConvexShape trapezoid(4);
    trapezoid.setPoint(0, { cx - topWidth, 0.f });
    trapezoid.setPoint(1, { cx + topWidth, 0.f });
    trapezoid.setPoint(2, { cx + topWidth * 0.67f, topPanelHeight });
    trapezoid.setPoint(3, { cx - topWidth * 0.67f, topPanelHeight });
    trapezoid.setFillColor(Colors::Theme::BackgroundDark);
    window.draw(trapezoid);

    std::string waveStr = "ВОЛНА " + std::to_string(wave + 1);
    sf::Text waveText(*mainFont, sf::String::fromUtf8(waveStr.begin(), waveStr.end()), 28);
    sf::FloatRect wtB = waveText.getLocalBounds();
    waveText.setOrigin({ wtB.position.x + wtB.size.x / 2.f, 0.f });
    waveText.setPosition({ cx, topPanelHeight / 2.f - wtB.size.y});
    window.draw(waveText);

    if (state == WaveState::Waiting || state == WaveState::Idle) {
        skipBtn.setPosition({ cx + 100.f, topPanelHeight / 2.f - 24.f });
        skipBtn.render(window);
    }

    // Нижняя панель
    int slotsCount = (int)towerSlots.size();
    float panelW = (2 + slotsCount) * 100.f;
    float startX = cx - panelW / 2.f;
    float panelBottom = ws.y;
    float panelTop = ws.y - 120.f;

    sf::ConvexShape hexagon(6);
    hexagon.setPoint(0, { cx - panelW / 2.f, panelBottom });
    hexagon.setPoint(1, { cx - panelW / 2.f - 25.f, ws.y - 60.f });
    hexagon.setPoint(2, { cx - panelW / 2.f, panelTop });
    hexagon.setPoint(3, { cx + panelW / 2.f, panelTop });
    hexagon.setPoint(4, { cx + panelW / 2.f + 25.f, ws.y - 60.f });
    hexagon.setPoint(5, { cx + panelW / 2.f, panelBottom });
    hexagon.setFillColor(Colors::Theme::BackgroundDark);
    window.draw(hexagon);

    // Ресурсы — использование кэшированной текстуры монет
    sf::Sprite coins(*coinTex);
    coins.setScale({ 0.625f, 0.625f });
    coins.setPosition({ startX + 15.f, ws.y - 105.f });
    window.draw(coins);

    sf::Text mText(*mainFont, std::to_string(money) + "$", 26);
    mText.setFillColor(Colors::Theme::TextMoney);
    mText.setPosition({ startX + (90.f - mText.getLocalBounds().size.x) / 2, ws.y - 40.f });
    window.draw(mText);

    // Магазин башен
    for (int i = 0; i < (int)towerSlots.size(); i++) {
        towerSlots[i].render(window);
        if (i == selectedTowerSlot) {
            sf::RectangleShape highlight({ 90.f, 100.f });
            highlight.setPosition(towerSlots[i].getGlobalBounds().position);
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(Colors::Theme::TextMoney);
            highlight.setOutlineThickness(2.f);
            window.draw(highlight);
        }
    }

    // Состояние базы — использование кэшированной текстуры сердца
    sf::Sprite heart(*heartTex);
    heart.setScale({ 0.625f, 0.625f });
    heart.setPosition({ startX + (slotsCount + 1) * 100.f + 15.f, ws.y - 105.f });
    window.draw(heart);

    sf::Text lText(*mainFont, std::to_string(lives), 26);
    lText.setFillColor(Colors::Theme::TextLives);
    lText.setPosition({ startX + (slotsCount + 1) * 100.f + (90.f - lText.getLocalBounds().size.x) / 2, ws.y - 40.f });
    window.draw(lText);

    speedBtn.render(window);

    if (showTowerMenu) {
        upgradeBtn.render(window);
        sellBtn.render(window);
    }
}

// Пробрасывает события во все кнопки HUD. Возвращает true, если клик попал в UI.
bool HUD::handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) {
    pauseRequested = false;
    skipRequested = false;

    // Инициализация координат "вне экрана"
    sf::Vector2f mousePos(-10000.f, -10000.f);
    bool hasCoords = false;

    // Извлекаем координаты только из релевантных событий
    if (const auto* m = event.getIf<sf::Event::MouseButtonPressed>()) { 
        mousePos = window.mapPixelToCoords(m->position, uiView); 
        hasCoords = true; 
    } else if (const auto* r = event.getIf<sf::Event::MouseButtonReleased>()) { 
        mousePos = window.mapPixelToCoords(r->position, uiView); 
        hasCoords = true; 
    } else if (const auto* mm = event.getIf<sf::Event::MouseMoved>()) { 
        mousePos = window.mapPixelToCoords(mm->position, uiView); 
        hasCoords = true; 
    } else if (const auto* tb = event.getIf<sf::Event::TouchBegan>()) { 
        mousePos = window.mapPixelToCoords(tb->position, uiView); 
        hasCoords = true; 
    } else if (const auto* te = event.getIf<sf::Event::TouchEnded>()) { 
        mousePos = window.mapPixelToCoords(te->position, uiView); 
        hasCoords = true; 
    }

    // Лямбда для проверки: попал ли клик в конкретную кнопку (только если есть координаты)
    auto hit = [&](const UI::Button& btn) {
        return hasCoords && btn.isVisible() && btn.getGlobalBounds().contains(mousePos);
    };

    bool hitUI = false;
    if (hit(pauseBtn)) hitUI = true;
    if (hit(skipBtn)) hitUI = true;
    if (hit(speedBtn)) hitUI = true;
    if (showTowerMenu) {
        if (hit(upgradeBtn)) hitUI = true;
        if (hit(sellBtn)) hitUI = true;
    }
    for (auto& slot : towerSlots) if (hit(slot)) hitUI = true;

    // Передаем события кнопкам
    pauseBtn.handleEvent(event, window, uiView);
    skipBtn.handleEvent(event, window, uiView);
    speedBtn.handleEvent(event, window, uiView);
    if (showTowerMenu) {
        upgradeBtn.handleEvent(event, window, uiView);
        sellBtn.handleEvent(event, window, uiView);
    }
    for (auto& slot : towerSlots) slot.handleEvent(event, window, uiView);

    return hitUI;
}

// Позиционирует кнопки улучшения и продажи под выбранной платформой
void HUD::showTowerControls(sf::Vector2f screenPos, int sellPrice, float worldZoom) {
    showTowerMenu = true;
    sellRequested = false;
    upgradeRequested = false;
    
    // Не сбрасываем lastClickedBtnId — сохраняем состояние двойного клика при переключении башен
    upgradeBtn.setIconColor(sf::Color::White);
    sellBtn.setIconColor(sf::Color::White);
    
    // Инвертированный зум мира (0.5 при приближении -> 2.0 масштаб объектов)
    float invZoom = 1.f / worldZoom;

    // Масштабирование 1-к-1: теперь кнопки уменьшаются/увеличиваются в точности как башни.
    float targetScale = invZoom;

    // Лимиты масштаба: расширяем диапазон, чтобы кнопки могли становиться меньше при отдалении.
#ifdef __ANDROID__
    targetScale = std::clamp(targetScale, 0.5f, 1.2f);
#else
    targetScale = std::clamp(targetScale, 0.4f, 1.4f);
#endif

    // Применяем масштаб: базовая иконка 0.375f (для 36px при иконке 96px)
    float targetIconScale = 0.375f * targetScale;
    upgradeBtn.setIconScale({ targetIconScale, targetIconScale });
    sellBtn.setIconScale({ targetIconScale, targetIconScale });

    float btnSize = 36.f * targetScale;
    upgradeBtn.setSize({ btnSize, btnSize });
    sellBtn.setSize({ btnSize, btnSize });

    // Позиционирование: максимально прижимаем к основанию башни.
    // Эффективная высота башни уменьшена
    float towerEffectiveHeight = 24.f * invZoom;
    float gap = 1.f * targetScale; 
    float totalOffsetY = towerEffectiveHeight + (btnSize / 2.f) + gap;

    float horizontalGap = 3.f * targetScale;

    // Установка позиций (screenPos — центр башни)
    upgradeBtn.setPosition({ screenPos.x - btnSize - horizontalGap / 2.f, screenPos.y + totalOffsetY - (btnSize / 2.f) });
    sellBtn.setPosition({ screenPos.x + horizontalGap / 2.f, screenPos.y + totalOffsetY - (btnSize / 2.f) });
}

// Скрывает кнопки управления башней
void HUD::hideTowerControls() {
    showTowerMenu = false;
    lastClickedBtnId = 0;
    upgradeBtn.setIconColor(sf::Color::White);
    sellBtn.setIconColor(sf::Color::White);
}

// Проверка запроса на продажу (вызывается из Game)
bool HUD::isSellRequested() const {
    return sellRequested;
}

// Проверка запроса на улучшение (заглушка)
bool HUD::isUpgradeRequested() const {
    return upgradeRequested;
}

// Получение множителя скорости игры
float HUD::getGameSpeed() const {
    if (speedMode == 1) return 2.0f;
    if (speedMode == 2) return 3.0f;
    return 1.0f;
}

// Получение индекса выбранного слота
int HUD::getSelectedSlot() const {
    return selectedTowerSlot;
}

// Проверка запроса на паузу
bool HUD::isPauseRequested() const {
    return pauseRequested;
}

// Проверка запроса на старт волны
bool HUD::isSkipRequested() const {
    return skipRequested;
}

// Сброс выделенного слота
void HUD::resetSelectedSlot() {
    selectedTowerSlot = -1;
}