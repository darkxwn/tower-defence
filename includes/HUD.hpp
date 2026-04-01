#pragma once 
#include <SFML/Graphics.hpp>
#include "WaveSystem.hpp"

class HUD {
private:
    float uiScale = 1.5f; // Коэффициент увеличения
    std::vector<sf::RectangleShape> towerSlots;

    sf::RectangleShape pauseBtn;
    sf::RectangleShape skipBtn;

    bool pauseClicked = false;
    bool skipClicked = false;

    int selectedTowerSlot = -1;

public:
    HUD();
    void render(sf::RenderWindow& window, int money, int lives, int wave, WaveState state);
    void handleClick(sf::Vector2f mousePos);
    void resetSelectedSlot();

    bool isPauseClicked() const;
    bool isSkipClicked() const;
    int getSelectedSlot() const;
};