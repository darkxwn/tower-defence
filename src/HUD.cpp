#include "HUD.hpp"
#include "ResourceManager.hpp"
#include "GameData.hpp"
#include "Colors.hpp"
#include <SFML/Graphics.hpp>

// Конструктор инициализирует кнопки и их логику
HUD::HUD() {
    auto& font = ResourceManager::getFont("main"); // основной шрифт

    // Инициализация управляющих кнопок
    pauseBtn = UI::Button(ResourceManager::get("icon-pause"), sf::Vector2f(48.f, 48.f));
    pauseBtn.setTransparent(true);
    pauseBtn.setCallback([this]() { pauseRequested = true; });

    skipBtn = UI::Button(ResourceManager::get("icon-skip"), sf::Vector2f(48.f, 48.f));
    skipBtn.setTransparent(true);
    skipBtn.setCallback([this]() { skipRequested = true; });

    // speedBtn — иконка в полный размер кнопки, без фона
    speedBtn = UI::Button(ResourceManager::get("icon-speed1"), sf::Vector2f(96.f, 96.f));
    speedBtn.setTransparent(true);
    speedBtn.setCallback([this]() {
        speedMode = (speedMode + 1) % 3;
        speedBtn.setTexture(ResourceManager::get("icon-speed" + std::to_string(speedMode + 1)));
    });

    // Создание слотов для башен
    auto towerNames = GameData::getTowerNames();
    towerSlots.reserve((int)towerNames.size());
    for (int i = 0; i < (int)towerNames.size(); i++) {
        int cost = GameData::getTower(towerNames[i]).cost;
        UI::Button slot(ResourceManager::get("tower-" + towerNames[i] + "-preview"), font, std::to_string(cost) + "$", sf::Vector2f(90.f, 100.f));
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
void HUD::updateLayout(sf::Vector2f viewSize) {
    float cx = viewSize.x / 2.f; // центр по горизонтали

    pauseBtn.setPosition({ 20.f, 20.f });
    skipBtn.setPosition({ cx + 150.f, 10.f });
    speedBtn.setPosition({ 15.0f, viewSize.y - 111.0f });

    float panelWidth = (2 + (int)towerSlots.size()) * 100.f;
    float startXPos = cx - panelWidth / 2.f;

    for (int i = 0; i < (int)towerSlots.size(); i++) {
        towerSlots[i].setPosition({ (startXPos + 100.f) + i * 100.f, viewSize.y - 110.f });
    }
}

// Отрисовывает интерфейс: панели, ресурсы, волны и кнопки
void HUD::render(sf::RenderWindow& window, int money, int lives, int wave, WaveState state) {
    auto& font = ResourceManager::getFont("main"); // шрифт
    sf::Vector2f ws = window.getView().getSize(); // логический размер UI
    float cx = ws.x / 2.f; // центр

    pauseBtn.render(window);

    // Верхняя панель волн
    sf::ConvexShape trapezoid;
    trapezoid.setPointCount(4);
    trapezoid.setPoint(0, { cx - 300.f, 0.f });
    trapezoid.setPoint(1, { cx + 300.f, 0.f });
    trapezoid.setPoint(2, { cx + 200.f, 75.f });
    trapezoid.setPoint(3, { cx - 200.f, 75.f });
    trapezoid.setFillColor(Colors::Theme::PanelBg);
    window.draw(trapezoid);

    std::string waveStr = "ВОЛНА " + std::to_string(wave + 1);
    sf::Text waveText(font, sf::String::fromUtf8(waveStr.begin(), waveStr.end()), 28);
    sf::FloatRect wtB = waveText.getLocalBounds();
    waveText.setOrigin({ wtB.position.x + wtB.size.x / 2.f, 0.f });
    waveText.setPosition({ cx, 15.f });
    window.draw(waveText);

    if (state == WaveState::Waiting || state == WaveState::Idle) {
        skipBtn.render(window);
    }

    // Нижняя панель башен
    int slotsCount = (int)towerSlots.size();
    float panelW = (2 + slotsCount) * 100.f;
    float startX = cx - panelW / 2.f;

    sf::ConvexShape hexagon;
    hexagon.setPointCount(6);
    hexagon.setPoint(0, { cx - panelW / 2.f, ws.y });
    hexagon.setPoint(1, { cx - panelW / 2.f - 25.f, ws.y - 60.f });
    hexagon.setPoint(2, { cx - panelW / 2.f, ws.y - 120.f });
    hexagon.setPoint(3, { cx + panelW / 2.f, ws.y - 120.f });
    hexagon.setPoint(4, { cx + panelW / 2.f + 25.f, ws.y - 60.f });
    hexagon.setPoint(5, { cx + panelW / 2.f, ws.y });
    hexagon.setFillColor(Colors::Theme::PanelBg);
    window.draw(hexagon);

    // Ресурсы
    sf::Sprite coins(ResourceManager::get("icon-coins"));
    coins.setScale({ 1.25f, 1.25f });
    coins.setPosition({ startX + 15.f, ws.y - 105.f });
    window.draw(coins);

    sf::Text mText(font, std::to_string(money) + "$", 26);
    mText.setFillColor(Colors::Theme::TextMoney);
    mText.setPosition({ startX + (90.f - mText.getLocalBounds().size.x) / 2, ws.y - 40.f });
    window.draw(mText);

    // Магазин башен
    auto towerNames = GameData::getTowerNames();
    for (int i = 0; i < (int)towerSlots.size(); i++) {
        towerSlots[i].render(window);
        sf::Vector2f slotPos = towerSlots[i].getGlobalBounds().position;

        if (i == selectedTowerSlot) {
            sf::RectangleShape highlight({ 90.f, 100.f });
            highlight.setPosition(slotPos);
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(Colors::Theme::TextMoney);
            highlight.setOutlineThickness(2.f);
            window.draw(highlight);
        }
    }

    // Состояние базы
    sf::Sprite heart(ResourceManager::get("icon-heart"));
    heart.setScale({ 1.25f, 1.25f });
    heart.setPosition({ startX + (slotsCount + 1) * 100.f + 15.f, ws.y - 105.f });
    window.draw(heart);

    sf::Text lText(font, std::to_string(lives), 26);
    lText.setFillColor(Colors::Theme::TextLives);
    lText.setPosition({ startX + (slotsCount + 1) * 100.f + (90.f - lText.getLocalBounds().size.x) / 2, ws.y - 40.f });
    window.draw(lText);

    speedBtn.render(window);
}

// Пробрасывает события во все кнопки HUD
void HUD::handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) {
    pauseRequested = false;
    skipRequested = false;

    pauseBtn.handleEvent(event, window, uiView);
    skipBtn.handleEvent(event, window, uiView);
    speedBtn.handleEvent(event, window, uiView);
    for (auto& slot : towerSlots) {
        slot.handleEvent(event, window, uiView);
    }
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