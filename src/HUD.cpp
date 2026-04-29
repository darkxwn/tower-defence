#include "HUD.hpp"
#include "ResourceManager.hpp"
#include "GameData.hpp"
#include "Colors.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>

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
        speedBtn.setTexture(*speedTexs[speedMode]);
    });

    // Инициализация кнопок управления башней
    upgradeBtn = UI::Button(ResourceManager::get("icon-upgrade"), *mainFont, "", sf::Vector2f(48.f, 48.f), UI::IconPlacement::Top);
    upgradeBtn.setIconScale({ 0.32f, 0.32f }); 
    upgradeBtn.setTextSize(11);
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

    sellBtn = UI::Button(ResourceManager::get("icon-sell"), *mainFont, "", sf::Vector2f(48.f, 48.f), UI::IconPlacement::Top);
    sellBtn.setIconScale({ 0.32f, 0.32f });
    sellBtn.setTextSize(11);
    sellBtn.setCallback([this]() { 
        float elapsed = doubleClickClock.getElapsedTime().asSeconds();
        if (lastClickedBtnId == 2 && elapsed < doubleClickTime) {
            sellRequested = true;
            lastClickedBtnId = 0;
            sellBtn.setIconColor(sf::Color::White);
        } else {
            lastClickedBtnId = 2;
            doubleClickClock.restart();
            sellBtn.setIconColor(Colors::Theme::TextRed);
            upgradeBtn.setIconColor(sf::Color::White);
        }
    });

    // Создание слотов для башен
    auto towerNames = GameData::getTowerNames();
    towerSlots.reserve((int)towerNames.size());
    for (int i = 0; i < (int)towerNames.size(); i++) {
        int cost = GameData::getTower(towerNames[i]).cost;
        UI::Button slot(ResourceManager::get("tower-" + towerNames[i] + "-preview"), *mainFont, std::to_string(cost) + "$", sf::Vector2f(90.f, 100.f), UI::IconPlacement::Top);
        
        // Установка фона карточки (9-slice)
        slot.setBackgroundTextures(
            &ResourceManager::get("panel"),
            &ResourceManager::get("panel-light"),
            &ResourceManager::get("panel-light"),
            &ResourceManager::get("panel"),
            24.0f
        );

        slot.setIconScale(sf::Vector2f(0.15625f, 0.15625f));
        slot.setTextSize(16);
        slot.setTextColor(Colors::Theme::TextYellow);
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
    float s = uiScale;

    float margin = std::round(20.f * s);
    pauseBtn.setPosition({ margin, margin });
    pauseBtn.setSize({ std::round(64.f * s), std::round(64.f * s) });
    
    speedBtn.setPosition({ std::round(15.0f * s), viewSize.y - std::round(111.0f * s) });
    speedBtn.setSize({ std::round(96.f * s), std::round(96.f * s) });

    float slotW = 100.f * s;
    float panelWidth = (2 + (int)towerSlots.size()) * slotW;
    float startX = std::round(cx - panelWidth / 2.f);
    float slotY = viewSize.y - std::round(110.f * s);

    for (int i = 0; i < (int)towerSlots.size(); i++) {
        float slotSizeW = 90.f * s;
        float slotSizeH = 100.f * s;
        towerSlots[i].setSize({ std::round(slotSizeW), std::round(slotSizeH) });
        // Центрируем карточку внутри её логической ячейки 100*s
        float posX = startX + slotW + i * slotW + (slotW - slotSizeW) / 2.f;
        towerSlots[i].setPosition({ std::round(posX), std::round(slotY) });
        towerSlots[i].setTextSize(static_cast<unsigned int>(16 * s));
    }
}

void HUD::setUiScale(float scale) {
    uiScale = scale;
}

// Отрисовывает интерфейс: панели, ресурсы, волны и кнопки
void HUD::render(sf::RenderWindow& window, int money, int lives, int wave, WaveState state) {
    if (!mainFont || !coinTex || !heartTex) return;

    sf::Vector2f ws = window.getView().getSize();
    float cx = ws.x / 2.f;
    float s = uiScale;

    pauseBtn.render(window);

    // Верхняя панель волн
    float topPanelHeight = std::round(85.f * s);
    float topWidth = std::clamp(300.f * s, 250.f, cx * 0.8f);
    sf::ConvexShape trapezoid(4);
    trapezoid.setPoint(0, { cx - topWidth, 0.f });
    trapezoid.setPoint(1, { cx + topWidth, 0.f });
    trapezoid.setPoint(2, { cx + topWidth * 0.67f, topPanelHeight });
    trapezoid.setPoint(3, { cx - topWidth * 0.67f, topPanelHeight });
    trapezoid.setFillColor(Colors::Theme::BackgroundDark);
    window.draw(trapezoid);

    std::string waveStr = "ВОЛНА " + std::to_string(wave + 1);
    sf::Text waveText(*mainFont, sf::String::fromUtf8(waveStr.begin(), waveStr.end()), static_cast<unsigned int>(28 * s));
    sf::FloatRect wtB = waveText.getLocalBounds();
    waveText.setOrigin({ wtB.position.x + wtB.size.x / 2.f, 0.f });
    waveText.setPosition({ cx, std::round(topPanelHeight / 2.f - wtB.size.y) });
    window.draw(waveText);

    if (state == WaveState::Waiting || state == WaveState::Idle) {
        skipBtn.setSize({ 48.f * s, 48.f * s });
        skipBtn.setPosition({ cx + 100.f * s, std::round(topPanelHeight / 2.f - 24.f * s) });
        skipBtn.render(window);
    }

    // Нижняя панель
    int slotsCount = (int)towerSlots.size();
    float slotW = 100.f * s;
    float panelW = (2 + slotsCount) * slotW;
    float startX = std::round(cx - panelW / 2.f);
    float panelBottom = ws.y;
    float panelTop = ws.y - std::round(120.f * s);

    // Подложка всей панели
    sf::ConvexShape hexagon(6);
    hexagon.setPoint(0, { startX, panelBottom });
    hexagon.setPoint(1, { startX - 25.f * s, ws.y - 60.f * s });
    hexagon.setPoint(2, { startX, panelTop });
    hexagon.setPoint(3, { startX + panelW, panelTop });
    hexagon.setPoint(4, { startX + panelW + 25.f * s, ws.y - 60.f * s });
    hexagon.setPoint(5, { startX + panelW, panelBottom });
    hexagon.setFillColor(Colors::Theme::BackgroundDark);
    window.draw(hexagon);

    // Ресурсы (в первой ячейке панели)
    sf::Sprite coins(*coinTex);
    coins.setScale({ 0.625f * s, 0.625f * s });
    coins.setPosition({ startX + 15.f * s, ws.y - std::round(105.f * s) });
    window.draw(coins);

    sf::Text mText(*mainFont, std::to_string(money) + "$", static_cast<unsigned int>(26 * s));
    mText.setFillColor(Colors::Theme::TextYellow);
    sf::FloatRect mtB = mText.getLocalBounds();
    mText.setPosition({ std::round(startX + (slotW - mtB.size.x) / 2.f), ws.y - std::round(40.f * s) });
    window.draw(mText);

    // Магазин башен
    for (int i = 0; i < (int)towerSlots.size(); i++) {
        towerSlots[i].render(window);

        if (i == selectedTowerSlot) {
            sf::RectangleShape highlight(towerSlots[i].getSize());
            highlight.setPosition(towerSlots[i].getPosition());
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(Colors::Theme::TextYellow);
            highlight.setOutlineThickness(2.f * s);
            window.draw(highlight);
        }
    }

    // Состояние базы (в последней ячейке панели)
    float heartStartX = startX + (slotsCount + 1) * slotW;
    sf::Sprite heart(*heartTex);
    heart.setScale({ 0.625f * s, 0.625f * s });
    heart.setPosition({ heartStartX + 15.f * s, ws.y - std::round(105.f * s) });
    window.draw(heart);

    sf::Text lText(*mainFont, std::to_string(lives), static_cast<unsigned int>(26 * s));
    lText.setFillColor(Colors::Theme::TextRed);
    sf::FloatRect ltB = lText.getLocalBounds();
    lText.setPosition({ std::round(heartStartX + (slotW - ltB.size.x) / 2.f), ws.y - std::round(40.f * s) });
    window.draw(lText);

    speedBtn.render(window);

    if (showTowerMenu) {
        if (canUpgrade) {
            upgradeBtn.setTextColor(money >= currentUpgradePrice ? Colors::Theme::TextGreen : sf::Color::Red);
        } else {
            upgradeBtn.setTextColor(sf::Color(150, 150, 150));
        }
        upgradeBtn.render(window);
        sellBtn.render(window);
    }
}

// Пробрасывает события во все кнопки HUD
bool HUD::handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) {
    pauseRequested = false;
    skipRequested = false;

    sf::Vector2f mousePos(-10000.f, -10000.f);
    bool hasCoords = false;

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
void HUD::showTowerControls(sf::Vector2f screenPos, int sellPrice, int upgradePrice, bool canUpgrade, float worldZoom) {
    showTowerMenu = true;
    this->currentSellPrice = sellPrice;
    this->currentUpgradePrice = upgradePrice;
    this->canUpgrade = canUpgrade;
    
    upgradeBtn.setEnabled(canUpgrade);
    upgradeBtn.setText(canUpgrade ? std::to_string(upgradePrice) + "$" : "MAX");
    upgradeBtn.setTextColor(canUpgrade ? Colors::Theme::TextGreen : sf::Color(150, 150, 150));
    
    sellBtn.setText(std::to_string(sellPrice) + "$");
    sellBtn.setTextColor(Colors::Theme::TextRed);
    
    upgradeBtn.setIconColor(sf::Color::White);
    sellBtn.setIconColor(sf::Color::White);
    
    float invZoom = 1.f / worldZoom;
    float targetScale = invZoom;

#ifdef __ANDROID__
    targetScale = std::clamp(targetScale, 0.7f, 1.2f);
#else
    targetScale = std::clamp(targetScale, 0.6f, 1.4f);
#endif

    float targetIconScale = 0.32f * targetScale;
    upgradeBtn.setIconScale({ targetIconScale, targetIconScale });
    sellBtn.setIconScale({ targetIconScale, targetIconScale });

    sf::Vector2f btnSize(48.f * targetScale, 48.f * targetScale);
    upgradeBtn.setSize(btnSize);
    sellBtn.setSize(btnSize);
    upgradeBtn.setTextSize(static_cast<unsigned int>(11 * targetScale));
    sellBtn.setTextSize(static_cast<unsigned int>(11 * targetScale));

    float towerHeightOffset = 32.f * invZoom; 
    float gap = 8.f * targetScale; 
    float totalOffsetY = towerHeightOffset + gap;

    float horizontalGap = 8.f * targetScale;

    upgradeBtn.setPosition({ screenPos.x - btnSize.x - horizontalGap / 2.f, screenPos.y + totalOffsetY });
    sellBtn.setPosition({ screenPos.x + horizontalGap / 2.f, screenPos.y + totalOffsetY });
}

void HUD::hideTowerControls() {
    showTowerMenu = false;
    lastClickedBtnId = 0;
    upgradeBtn.setIconColor(sf::Color::White);
    sellBtn.setIconColor(sf::Color::White);
}

bool HUD::isSellRequested() const { return sellRequested; }
bool HUD::isUpgradeRequested() const { return upgradeRequested; }

void HUD::resetRequests() {
    sellRequested = false;
    upgradeRequested = false;
}

float HUD::getGameSpeed() const {
    if (speedMode == 1) return 2.0f;
    if (speedMode == 2) return 3.0f;
    return 1.0f;
}

int HUD::getSelectedSlot() const { return selectedTowerSlot; }
bool HUD::isPauseRequested() const { return pauseRequested; }
bool HUD::isSkipRequested() const { return skipRequested; }
void HUD::resetSelectedSlot() { selectedTowerSlot = -1; }
