#pragma once
#include <SFML/Graphics/Color.hpp>

///////////////////////////////////////////////////////////////////////////
//
// ПАЛИТРА И ТЕМЫ ОФОРМЛЕНИЯ
//
///////////////////////////////////////////////////////////////////////////

namespace Colors {
    namespace Palette {
        inline const sf::Color Gray5{ 242, 242, 242 };  // 5% серого
        inline const sf::Color Gray10{ 230, 230, 230 }; // 10% серого
        inline const sf::Color Gray20{ 204, 204, 204 }; // 20% серого
        inline const sf::Color Gray30{ 179, 179, 179 }; // 30% серого
        inline const sf::Color Gray40{ 153, 153, 153 }; // 40% серого
        inline const sf::Color Gray50{ 128, 128, 128 }; // 50% серого
        inline const sf::Color Gray60{ 102, 102, 102 }; // 60% серого
        inline const sf::Color Gray70{ 77, 77, 77 };    // 70% серого
        inline const sf::Color Gray80{ 51, 51, 51 };    // 80% серого
        inline const sf::Color Gray90{ 26, 26, 26 };    // 90% серого

        inline const sf::Color White{ 255, 255, 255 };  // чистый белый
        inline const sf::Color Black{ 0, 0, 0 };        // чистый черный

        inline const sf::Color Red{ 255, 0, 0 };        // ярко-красный

        inline const sf::Color Gold{ 255, 212, 42 };    // золото для денег
        inline const sf::Color Crimson{ 255, 42, 42 };  // малиновый для жизней
        inline const sf::Color WashedGreen{ 42, 255, 42};
    }

    namespace Theme {
        inline const sf::Color PanelBg = Palette::Black;           // фон панелей
        inline const sf::Color WidgetBg = sf::Color(61, 61, 61, 200); // фон виджетов
        inline const sf::Color WidgetHover = sf::Color(72, 72, 72, 230); // фон при наведении
        inline const sf::Color WidgetDisabled = sf::Color(35, 35, 35, 140); // фон выключенного виджета

        inline const sf::Color TextMain = Palette::White;           // основной текст
        inline const sf::Color TextMoney = Palette::Gold;           // текст денег
        inline const sf::Color TextLives = Palette::Crimson;        // текст жизней
        inline const sf::Color TextVersion = sf::Color(120, 120, 120, 200); // текст версии

        inline const sf::Color RadiusFill = sf::Color(255, 255, 255, 20); // заливка радиуса башни
        inline const sf::Color RadiusOutline = sf::Color(255, 255, 255, 80); // контур радиуса башни
        inline const sf::Color HpBarBg = Palette::Gray70;           // фон полоски здоровья
        inline const sf::Color HpBarFill = Palette::Crimson;        // заполнение здоровья
    }
}