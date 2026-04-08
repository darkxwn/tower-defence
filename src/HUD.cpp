#include "HUD.hpp"
#include "ResourceManager.hpp"
#include "GameData.hpp"
#include <SFML/Graphics.hpp>
#include "Colors.hpp"

HUD::HUD() {
    pauseBtn.setSize({ 48.f, 48.f });
    pauseBtn.setFillColor(sf::Color::Transparent);

    skipBtn.setSize({ 48.f, 48.f });
    skipBtn.setFillColor(sf::Color::Transparent);

    speedBtn.setSize({ 96.f, 96.f });
    speedBtn.setFillColor(sf::Color::Transparent);

    for (int i = 0; i < 4; i++) {
        sf::RectangleShape slot({ 90.f, 100.f });
        slot.setFillColor(Colors::slotBg);
        towerSlots.push_back(slot);
    }
}

void HUD::updateLayout(sf::Vector2f viewSize) {
    pauseBtn.setPosition({ 10.f, 10.f });

    // skipBtn позиционируется в render динамически, 
    // но для handleClick нам нужно актуальное положение:
    float cx = viewSize.x / 2.f;
    skipBtn.setPosition({ cx + 150.f, 10.f });

    // Кнопка скорости (слева внизу)
    speedBtn.setPosition({ 15.0f, viewSize.y - (96.0f + 15.0f) });

    // Обновляем позиции слотов башен (они тоже зависят от центра экрана)
    int slots = 2 + (int)towerSlots.size();
    float panelWidth = slots * 100.f;
    float startXPos = cx - panelWidth / 2.f;
    for (int i = 0; i < (int)towerSlots.size(); i++) {
        towerSlots[i].setPosition({ (startXPos + 100.f) + i * 100.f, viewSize.y - 110.f });
    }
}

void HUD::render(sf::RenderWindow& window, int money, int lives, int wave, WaveState state) {
    auto& font = ResourceManager::getFont("main");
    sf::Vector2f winSize = window.getView().getSize();

    float viewW = winSize.x;
    float viewH = winSize.y;

    float cx = winSize.x / 2.f;

    window.draw(pauseBtn);
    sf::Sprite pauseIcon(ResourceManager::get("icon-pause"));
    pauseIcon.setPosition(pauseBtn.getPosition());
    window.draw(pauseIcon);

    sf::ConvexShape trapezoid;
    trapezoid.setPointCount(4);
    trapezoid.setPoint(0, { cx - 300.f, 0.f });
    trapezoid.setPoint(1, { cx + 300.f, 0.f });
    trapezoid.setPoint(2, { cx + 200.f, 75.f });
    trapezoid.setPoint(3, { cx - 200.f, 75.f });
    trapezoid.setFillColor(Colors::panelBg);
    window.draw(trapezoid);

    std::string waveStr = "ВОЛНА " + std::to_string(wave + 1);
    sf::Text waveText(font, sf::String::fromUtf8(waveStr.begin(), waveStr.end()), 28);
    waveText.setFillColor(sf::Color::White);
    float waveTextX = cx - waveText.getLocalBounds().size.x / 2.f;
    waveText.setPosition({ waveTextX, 15.f });
    window.draw(waveText);

    if (state == WaveState::Waiting || state == WaveState::Idle) {
        skipBtn.setPosition({ cx + 150.f, 10.f });
        window.draw(skipBtn);
        sf::Sprite skipIcon(ResourceManager::get("icon-skip"));
        skipIcon.setPosition(skipBtn.getPosition());
        window.draw(skipIcon);
    }

    int slots = 2 + (int)towerSlots.size();
    float panelWidth = slots * 100.f;
    float startXPos = cx - panelWidth / 2.f;

    sf::ConvexShape hexagon;
    float hexagonWidth = slots * 100.f;
    float hexagonHeight = 120.f;
    hexagon.setPointCount(6);
    hexagon.setPoint(0, { cx - hexagonWidth / 2.f, (float)winSize.y  });
    hexagon.setPoint(1, { cx - hexagonWidth / 2.f - 25.f, (float)winSize.y  - hexagonHeight / 2.f });
    hexagon.setPoint(2, { cx - hexagonWidth / 2.f, (float)winSize.y  - hexagonHeight });
    hexagon.setPoint(3, { cx + hexagonWidth / 2.f, (float)winSize.y  - hexagonHeight });
    hexagon.setPoint(4, { cx + hexagonWidth / 2.f + 25.f, (float)winSize.y  - hexagonHeight / 2.f });
    hexagon.setPoint(5, { cx + hexagonWidth / 2.f, (float)winSize.y  });
    hexagon.setFillColor(Colors::panelBg);
    window.draw(hexagon);
    
    // иконка денег
    sf::Sprite coinsIcon(ResourceManager::get("icon-coins"));
    coinsIcon.setScale({ 1.25f, 1.25f });
    coinsIcon.setPosition({ startXPos + 15.f, (float)winSize.y - 105.f  });
    window.draw(coinsIcon);
    // текст денег
    sf::Text moneyText(font, std::to_string(money) + "$", 26);
    moneyText.setFillColor(Colors::moneyText);
    moneyText.setPosition({ startXPos + (90.f - moneyText.getLocalBounds().size.x) / 2, (float)winSize.y - 40.f  });
    window.draw(moneyText);

    auto towerNames = GameData::getTowerNames();
    for (int i = 0; i < (int)towerSlots.size(); i++) {
        towerSlots[i].setPosition({ (startXPos + 100.f) + i * 100.f, (float)winSize.y - 110.f  });
        window.draw(towerSlots[i]);

        // подсветка башни в хотбаре
        if (i == selectedTowerSlot) {
            sf::RectangleShape highlight({ 90.f, 100.f });
            highlight.setPosition(towerSlots[i].getPosition());
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(Colors::moneyText);
            highlight.setOutlineThickness(2.f);
            window.draw(highlight);
        }

        if (i < (int)towerNames.size()) {
            // иконка башни
            sf::Sprite preview(ResourceManager::get("tower-" + towerNames[i] + "-preview"));
            preview.setScale({ 0.15625f, 0.15625f });
            auto slotPos = towerSlots[i].getPosition();
            preview.setPosition(slotPos + sf::Vector2f(5.f, 0.f));
            window.draw(preview);
            // цена башни
            int cost = GameData::getTower(towerNames[i]).cost;
            sf::Text costText(font, std::to_string(cost) + "$", 18);
            costText.setFillColor(Colors::moneyText);
            costText.setPosition(slotPos + sf::Vector2f((90.f - costText.getLocalBounds().size.x) / 2, 75.f));
            window.draw(costText);
        }
    }
    // иконка жизней
    sf::Sprite heartIcon(ResourceManager::get("icon-heart"));
    heartIcon.setScale({ 1.25f, 1.25f });
    heartIcon.setPosition({ startXPos + ((int)towerSlots.size() + 1) * 100.f + 15.f, (float)winSize.y - 105.f  });
    window.draw(heartIcon);
    // текст жизней
    sf::Text livesText(font, std::to_string(lives), 26);
    livesText.setFillColor(Colors::livesText);
    livesText.setPosition({ startXPos + ((int)towerSlots.size() + 1) * 100.f + (90.f - livesText.getLocalBounds().size.x) / 2, (float)winSize.y - 40.f  });
    window.draw(livesText);

    // иконка скорости
    std::string speedTex = "icon-speed" + std::to_string(speedMode + 1);
    sf::Sprite sBtnIcon(ResourceManager::get(speedTex));
    sBtnIcon.setPosition({ 12.0f, winSize.y - 96.0f - 12.0f });
    window.draw(sBtnIcon);

    // текста скорости
    std::string speedLabels[] = { "x1", "x2", "x3" };
    sf::Text sText(font, speedLabels[speedMode], 22);
    sText.setFillColor(sf::Color::White);
    // центрируем текст
    sf::FloatRect tBounds = sText.getLocalBounds();
    sText.setOrigin({ tBounds.size.x / 2.f, tBounds.size.y / 2.f });
    sText.setPosition({ 12.0f + 48.f, winSize.y - 96.0f + 44.f - 12.0f});
    window.draw(sText);
}

void HUD::handleClick(sf::Vector2f mousePos) {
    pauseClicked = false;
    skipClicked = false;
    if (pauseBtn.getGlobalBounds().contains(mousePos)) { pauseClicked = true; return; }
    if (skipBtn.getGlobalBounds().contains(mousePos)) { skipClicked = true;  return; }
    for (int i = 0; i < (int)towerSlots.size(); i++) {
        if (towerSlots[i].getGlobalBounds().contains(mousePos)) {
            selectedTowerSlot = (selectedTowerSlot == i) ? -1 : i;
            return;
        }
    }
    // Проверка нажатия на кнопку скорости
    if (speedBtn.getGlobalBounds().contains(mousePos)) {
        speedMode = (speedMode + 1) % 3;
        return;
    }
}

// Возвращает коэффициент ускорения
float HUD::getGameSpeed() const {
    if (speedMode == 1) return 2.0f;
    if (speedMode == 2) return 3.0f;
    return 1.0f;
}

int HUD::getSelectedSlot() const { return selectedTowerSlot; }
bool HUD::isPauseClicked() const { return pauseClicked; }
bool HUD::isSkipClicked() const { return skipClicked; }
void HUD::resetSelectedSlot() { selectedTowerSlot = -1; }