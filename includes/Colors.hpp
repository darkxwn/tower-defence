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
        inline const sf::Color Gray95{ 13, 13, 13 };    // 95% серого

        inline const sf::Color Black{ 0, 0, 0 };        // чистый черный
        inline const sf::Color White{ 255, 255, 255 };  // чистый белый

        inline const sf::Color PastelYellow{ 255, 212, 42 }; // желтый
        inline const sf::Color PastelRed{ 255, 42, 42 };     // красный
        inline const sf::Color PastelGreen{ 42, 255, 42};    // зеленый
    }

    namespace Theme {

        inline const sf::Color Overlay = sf::Color(0, 0, 0, 200); // фон оверлеев
        inline const sf::Color BackgroundDark = Palette::Black; // темный фон панелей
        inline const sf::Color Background = Palette::Gray90; // основной фон

        inline const sf::Color Widget = Palette::Gray80; // фон виджетов
        inline const sf::Color WidgetHover = Palette::Gray70; // фон при наведении
        inline const sf::Color WidgetDisabled = Palette::Gray95; // фон выключенного виджета

        inline const sf::Color TextMain = Palette::White; // основной текст
        inline const sf::Color TextYellow = Palette::PastelYellow; // текст денег
        inline const sf::Color TextGreen = Palette::PastelGreen;
        inline const sf::Color TextRed = Palette::PastelRed; // текст жизней
        inline const sf::Color TextDark = Palette::Gray50; // темный текст 

        inline const sf::Color RadiusFill = sf::Color(255, 255, 255, 20); // заливка радиуса башни
        inline const sf::Color RadiusOutline = sf::Color(255, 255, 255, 80); // контур радиуса башни
        inline const sf::Color HpBarBg = Palette::Gray70;    // фон полоски здоровья
        inline const sf::Color HpBarFill = Palette::PastelRed; // заполнение здоровья


    }
}