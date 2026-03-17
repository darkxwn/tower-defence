#include "HUD.hpp"
#include "ResourceManager.hpp"
#include "Base.hpp"
#include <SFML/Graphics.hpp>
#include "Colors.hpp"

HUD::HUD(const std::string& fontPath) {
    if (!font.openFromFile(fontPath))
        throw std::runtime_error("[Ошибка]: Ошибка загрузки шрифта " + fontPath);

    // кнопка паузы — верхний левый угол, позиция фиксирована
    pauseBtn.setSize({ 48.f, 48.f });
    pauseBtn.setPosition({ 10.f, 10.f });
    pauseBtn.setFillColor(sf::Color::Transparent);

    // кнопка запуска волны — позиция считается в render динамически
    skipBtn.setSize({ 48.f, 48.f });
    skipBtn.setFillColor(sf::Color::Transparent);

    // слоты башен в нижней панели
    for (int i = 0; i < 4; i++) {
        sf::RectangleShape slot({ 90.f, 100.f });
        slot.setFillColor(Colors::slotBg);
        towerSlots.push_back(slot);
    }
}

void HUD::render(sf::RenderWindow& window, int money, int lives, int wave, WaveState state) {
    auto winSize = window.getSize();
    float cx = winSize.x / 2.f;  // центр по X

    // кнопка паузы
    window.draw(pauseBtn);
    sf::Sprite pauseIcon(ResourceManager::get("icon-pause"));
    pauseIcon.setPosition(pauseBtn.getPosition());
    window.draw(pauseIcon);

    // верхний блок - фон 
    sf::ConvexShape trapezoid;
    trapezoid.setPointCount(4);
    trapezoid.setPoint(0, { cx - 300.f, 0.f });
    trapezoid.setPoint(1, { cx + 300.f, 0.f });
    trapezoid.setPoint(2, { cx + 200.f, 75.f });
    trapezoid.setPoint(3, { cx - 200.f, 75.f });
    trapezoid.setFillColor(Colors::panelBg);
    window.draw(trapezoid);
    // верхний блок — номер волны 
    std::string waveStr = "ВОЛНА " + std::to_string(wave + 1);
    sf::Text waveText(font, sf::String::fromUtf8(waveStr.begin(), waveStr.end()), 28);
    waveText.setFillColor(sf::Color::White);
    float waveTextX = cx - waveText.getLocalBounds().size.x / 2.f;
    waveText.setPosition({ waveTextX, 15.f });
    window.draw(waveText);

    // кнопка скипа — только в паузе между волнами 
    if (state == WaveState::Waiting || state == WaveState::Idle) {
        skipBtn.setPosition({ cx + 150.f, 10.f });
        window.draw(skipBtn);
        sf::Sprite skipIcon(ResourceManager::get("icon-skip"));
        skipIcon.setPosition(skipBtn.getPosition());
        window.draw(skipIcon);
    }

    // нижняя панель 
    int slots = 2 + (int)towerSlots.size();
    float panelWidth = slots * 100.f;
    float startXPos = cx - panelWidth / 2.f;

    // фон нижней панели
    //  /``````````````\
    //  \______________/
    sf::ConvexShape hexagon;
    float hexagonWidth = slots * 100.f;
    float hexagonHeight = 120.f;
    hexagon.setPointCount(6);
    hexagon.setPoint(0, { cx - hexagonWidth / 2.f, (float)winSize.y });
    hexagon.setPoint(1, { cx - hexagonWidth / 2.f - 25.f, (float)winSize.y - hexagonHeight / 2.f });
    hexagon.setPoint(2, { cx - hexagonWidth / 2.f, (float)winSize.y - hexagonHeight });
    hexagon.setPoint(3, { cx + hexagonWidth / 2.f , (float)winSize.y - hexagonHeight });
    hexagon.setPoint(4, { cx + hexagonWidth / 2.f + 25.f, (float)winSize.y - hexagonHeight / 2.f });
    hexagon.setPoint(5, { cx + hexagonWidth / 2.f, (float)winSize.y });
    hexagon.setFillColor(Colors::panelBg);
    window.draw(hexagon);

    // ячейка с деньгами
    sf::Sprite coinsIcon(ResourceManager::get("icon-coins"));
    coinsIcon.setScale({ 1.25f, 1.25f });
    coinsIcon.setPosition({ startXPos + 15.f, (float)winSize.y - 105.f });
    window.draw(coinsIcon);
    // текущее количество денег
    sf::Text moneyText(font, std::to_string(money) + "$", 26);
    moneyText.setFillColor(Colors::moneyText);
    moneyText.setPosition({ startXPos + (90.f - moneyText.getLocalBounds().size.x) / 2, (float)winSize.y - 40.f });
    window.draw(moneyText);

    // слоты башен — позиции считаем динамически
    std::vector<std::string> towerNames = { "basic", "cannon", "double", "sniper" };
    std::vector<int> towerCosts = { 50, 100, 75, 120 };

    for (int i = 0; i < (int)towerSlots.size(); i++) {
        // позиция слота — рядом с ячейкой денег
        towerSlots[i].setPosition({ (startXPos + 100.f) + i * 100.f, (float)winSize.y - 110.f });
        window.draw(towerSlots[i]);

        if (i == selectedTowerSlot) {
            sf::RectangleShape highlight({ 90.f, 100.f });
            highlight.setPosition(towerSlots[i].getPosition());
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(sf::Color(255, 200, 37, 255));
            highlight.setOutlineThickness(2.f);
            window.draw(highlight);
        }

        // превью башни
        sf::Sprite preview(ResourceManager::get("tower-" + towerNames[i] + "-preview"));
        preview.setScale({ 1.25f, 1.25f });
        auto slotPos = towerSlots[i].getPosition();
        preview.setPosition(slotPos + sf::Vector2f(5.f, 0.f));
        window.draw(preview);

        // цена под спрайтом
        sf::Text costText(font, std::to_string(towerCosts[i]) + "$", 18);
        costText.setFillColor(sf::Color(255, 200, 37, 255));
        costText.setPosition(slotPos + sf::Vector2f((90.f - costText.getLocalBounds().size.x)/2, 70.f));
        window.draw(costText);
    }
    
    // ячейка с жизнями
    sf::Sprite heartIcon(ResourceManager::get("icon-heart"));
    heartIcon.setScale({ 1.25f, 1.25f });
    heartIcon.setPosition({ startXPos + ((int)towerSlots.size() + 1) * 100.f + 15.f, (float)winSize.y - 105.f });
    window.draw(heartIcon);
    // текущее количество жизней
    sf::Text livesText(font, std::to_string(lives), 26);
    livesText.setFillColor(Colors::livesText);
    livesText.setPosition({ startXPos + ((int)towerSlots.size() + 1) * 100.f + (90.f - livesText.getLocalBounds().size.x) / 2, (float)winSize.y - 40.f });
    window.draw(livesText);
}

void HUD::handleClick(sf::Vector2f mousePos) {
    pauseClicked = false;
    skipClicked = false;
    if (pauseBtn.getGlobalBounds().contains(mousePos)) {
        pauseClicked = true;
        return;
    }
    if (skipBtn.getGlobalBounds().contains(mousePos)) {
        skipClicked = true;
        return;
    }
    // проверяем клики по слотам башен
    for (int i = 0; i < (int)towerSlots.size(); i++) {
        if (towerSlots[i].getGlobalBounds().contains(mousePos)) {
            selectedTowerSlot = (selectedTowerSlot == i) ? -1 : i;
            return;
        }
    }
}

int HUD::getSelectedSlot() const { return selectedTowerSlot; }

bool HUD::isPauseClicked() const { return pauseClicked; }
bool HUD::isSkipClicked() const { return skipClicked; }