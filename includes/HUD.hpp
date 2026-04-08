#pragma once 
#include <SFML/Graphics.hpp>
#include "WaveSystem.hpp"

///////////////////////////////////////////////////////////////////////////
//
// КЛАСС HUD
//
///////////////////////////////////////////////////////////////////////////

class HUD {
private:
    float uiScale = 1.5f; // Коэффициент увеличения
    std::vector<sf::RectangleShape> towerSlots;

    sf::RectangleShape pauseBtn;
    sf::RectangleShape skipBtn;

    sf::RectangleShape speedBtn;
    int speedMode = 0; // 0 = x1, 1 = x2, 2 = x3

    bool pauseClicked = false;
    bool skipClicked = false;

    int selectedTowerSlot = -1;

public:
    HUD();
    void render(sf::RenderWindow& window, int money, int lives, int wave, WaveState state);
    void updateLayout(sf::Vector2f viewSize);
    void handleClick(sf::Vector2f mousePos);
    void resetSelectedSlot();

    float getGameSpeed() const;

    bool isPauseClicked() const;
    bool isSkipClicked() const;
    int getSelectedSlot() const;
};