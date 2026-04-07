#pragma once
#include <SFML/Graphics.hpp>

///////////////////////////////////////////////////////////////////////////
//
// ЦВЕТОВЫЕ КОНСТАНТЫ
//
///////////////////////////////////////////////////////////////////////////

namespace Colors {
    // UI
    inline const sf::Color panelBg = sf::Color(19, 19, 19, 255);
    inline const sf::Color slotBg = sf::Color(61, 61, 61, 200);
    inline const sf::Color moneyText = sf::Color(255, 200, 37, 255);
    inline const sf::Color livesText = sf::Color(234, 50, 60, 255);

    // Башни
    inline const sf::Color radiusFill = sf::Color(255, 255, 255, 20);
    inline const sf::Color radiusOutline = sf::Color(255, 255, 255, 80);

    // Враги
    inline const sf::Color hpBarBg = sf::Color(77, 77, 77, 255);
    inline const sf::Color hpBarFill = sf::Color(255, 42, 42, 255);

    // Фон игры
    inline const sf::Color gameBg = sf::Color(26, 26, 26, 255);
    
}