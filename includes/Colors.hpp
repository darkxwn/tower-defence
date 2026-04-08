#pragma once
#include <SFML/Graphics.hpp>

///////////////////////////////////////////////////////////////////////////
//
// ЦВЕТОВЫЕ КОНСТАНТЫ
//
///////////////////////////////////////////////////////////////////////////

namespace Colors {
    // ЦВЕТА НОВОЙ ПАЛИТРЫ
    // Оттенки серого (число - процент серого)
    // inline const sf::Color gray5 = sf::Color(242, 242, 242, 255);
    // inline const sf::Color gray10 = sf::Color(230, 230, 230, 255);
    // inline const sf::Color gray20 = sf::Color(204, 204, 204, 255);
    // inline const sf::Color gray30 = sf::Color(179, 179, 179, 255);
    // inline const sf::Color gray40 = sf::Color(153, 153 ,153, 255);
    // inline const sf::Color gray50 = sf::Color(128, 128, 128, 255);
    // inline const sf::Color gray60 = sf::Color(102, 102, 102, 255);
    // inline const sf::Color gray70 = sf::Color(77, 77, 77, 255);
    // inline const sf::Color gray80 = sf::Color(51, 51, 51, 255);
    // inline const sf::Color gray90 = sf::Color(26, 26, 26, 255);

    // Белый
    // inline const sf::Color white = sf::Color(255, 255, 255, 255);
    // Черный
    // inline const sf::Color black = sf::Color(0, 0, 0, 255);

    // Ярко красный
    // inline const sf::Color red = sf::Color(255, 0, 0, 255);

    // Ярко желтый
    // inline const sf::Color red = sf::Color(255, 255, 0, 255);

    // Ярко зеленый
    // inline const sf::Color red = sf::Color(0, 255, 0, 255);

    // Ярко синий
    // inline const sf::Color red = sf::Color(0, 0, 255, 255);
    
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