#include "ui/Container.hpp"

namespace UI {

    ///////////////////////////////////////////////////////////////////////////
    //
    // РЕАЛИЗАЦИЯ КЛАССА CONTAINER
    //
    ///////////////////////////////////////////////////////////////////////////

    // Конструктор инициализирует базовые фигуры и цвета
    Container::Container(sf::Vector2f size) {
        this->size = size;

        // настройка параметров фона
        background.setSize(size);
        background.setFillColor(bgColor);

        // настройка параметров отладочной рамки
        outline.setSize(size);
        outline.setFillColor(sf::Color::Transparent);
        outline.setOutlineColor(sf::Color::Red);
        outline.setOutlineThickness(1.f);
    }

    // Изменение позиции контейнера и его содержимого
    void Container::setPosition(sf::Vector2f pos) {
        position = pos;
        background.setPosition(pos);
        outline.setPosition(pos);
        recalculateLayout();
    }

    // Изменение размера контейнера и пересчет раскладки
    void Container::setSize(sf::Vector2f size) {
        this->size = size;
        background.setSize(size);
        outline.setSize(size);
        recalculateLayout();
    }

    // Изменение направления и немедленный пересчет
    void Container::setDirection(Direction dir) {
        direction = dir;
        recalculateLayout();
    }

    // Изменение выравнивания контента по главной оси
    void Container::setContentAlign(ContentAlign align) {
        contentAlign = align;
        recalculateLayout();
    }

    // Изменение выравнивания элементов по поперечной оси
    void Container::setItemAlign(ItemAlign align) {
        itemAlign = align;
        recalculateLayout();
    }

    // Изменение режима переноса элементов
    void Container::setWrap(bool value) {
        wrap = value;
        recalculateLayout();
    }

    // Изменение отступа между элементами
    void Container::setGap(float value) {
        gap = value;
        recalculateLayout();
    }

    // Изменение внутренних отступов контейнера
    void Container::setPadding(sf::Vector2f value) {
        padding = value;
        recalculateLayout();
    }

    // Изменение цвета заливки фона
    void Container::setBackgroundColor(sf::Color color) {
        bgColor = color;
        background.setFillColor(bgColor);
    }

    // Изменение флага видимости фона
    void Container::setDrawBackground(bool draw) {
        drawBackground = draw;
    }

    // Изменение флага видимости отладочной рамки
    void Container::setDrawOutline(bool draw) {
        drawOutline = draw;
    }

    // Изменение активности прокрутки
    void Container::setScrollEnabled(bool enabled) {
        scrollEnabled = enabled;
        if (!enabled) scrollOffset = 0.f;
        recalculateLayout();
    }

    // Добавление виджета в контейнер
    void Container::addChild(std::unique_ptr<Widget> child) {
        children.push_back(std::move(child));
        recalculateLayout();
    }

    // Удаление виджета по индексу
    void Container::removeChild(size_t index) {
        if (index < children.size()) {
            children.erase(children.begin() + index);
            recalculateLayout();
        }
    }

    // Удаление всех дочерних элементов
    void Container::clearChildren() {
        children.clear();
        scrollOffset = 0.f;
        recalculateLayout();
    }

    // Получение указателя на виджет по индексу
    Widget* Container::getChild(size_t index) const {
        if (index < children.size()) {
            return children[index].get();
        }
        return nullptr;
    }

    // Получение текущего количества детей
    size_t Container::getChildrenCount() const {
        return children.size();
    }

    // Принудительный пересчет позиций
    void Container::rebuild() {
        recalculateLayout();
    }

    // Расчет позиций дочерних элементов на основе Flexbox модели
    void Container::recalculateLayout() {
        maxContentHeight = 0.f;
        
        // сбор элементов, участвующих в автоматической расстановке
        std::vector<Widget*> layoutChildren;
        for (auto& child : children) {
            if (child->isVisible() && child->getFollowsLayout()) {
                layoutChildren.push_back(child.get());
            }
        }

        if (layoutChildren.empty()) return;

        // структура для управления линиями при переносе (Wrap)
        struct Line {
            std::vector<Widget*> items; 
            float mainSize = 0.f; 
            float crossSize = 0.f; 
        };

        std::vector<Line> lines;
        lines.push_back(Line());

        float maxMainSize = (direction == Direction::Row) ? (size.x - padding.x * 2.f) : (size.y - padding.y * 2.f);

        // распределение элементов по линиям
        for (auto* item : layoutChildren) {
            sf::Vector2f itemSize = item->getSize();
            float itemMain = (direction == Direction::Row) ? itemSize.x : itemSize.y;
            float itemCross = (direction == Direction::Row) ? itemSize.y : itemSize.x;

            Line& currentLine = lines.back();
            float spacing = currentLine.items.empty() ? 0.f : gap;

            if (wrap && !currentLine.items.empty() && (currentLine.mainSize + spacing + itemMain > maxMainSize)) {
                lines.push_back(Line());
                Line& newLine = lines.back();
                newLine.items.push_back(item);
                newLine.mainSize = itemMain;
                newLine.crossSize = itemCross;
            }
            else {
                currentLine.items.push_back(item);
                currentLine.mainSize += spacing + itemMain;
                if (itemCross > currentLine.crossSize) currentLine.crossSize = itemCross;
            }
        }

        // расчет общего размера всех линий по поперечной оси
        float totalLinesCrossSize = (lines.size() - 1) * gap;
        for (const auto& line : lines) totalLinesCrossSize += line.crossSize;

        // определение начальной позиции по поперечной оси
        float currentCrossPos = 0.f;
        float containerCrossSize = (direction == Direction::Row) ? (size.y - padding.y * 2.f) : (size.x - padding.x * 2.f);

        if (contentAlign == ContentAlign::Center) {
            currentCrossPos = std::max(0.f, (containerCrossSize - totalLinesCrossSize) / 2.f);
        }
        else if (contentAlign == ContentAlign::End) {
            currentCrossPos = std::max(0.f, containerCrossSize - totalLinesCrossSize);
        }

        // позиционирование каждой линии и элементов внутри нее
        for (const auto& line : lines) {
            float currentMainPos = 0.f;

            if (contentAlign == ContentAlign::Center) {
                currentMainPos = std::max(0.f, (maxMainSize - line.mainSize) / 2.f);
            }
            else if (contentAlign == ContentAlign::End) {
                currentMainPos = std::max(0.f, maxMainSize - line.mainSize);
            }

            for (auto* item : line.items) {
                sf::Vector2f itemSize = item->getSize();
                float itemCross = (direction == Direction::Row) ? itemSize.y : itemSize.x;
                float itemCrossOffset = 0.f;

                if (itemAlign == ItemAlign::Center) {
                    itemCrossOffset = (line.crossSize - itemCross) / 2.f;
                }
                else if (itemAlign == ItemAlign::End) {
                    itemCrossOffset = line.crossSize - itemCross;
                }

                float finalX = 0.f;
                float finalY = 0.f;

                if (direction == Direction::Row) {
                    finalX = padding.x + currentMainPos;
                    finalY = padding.y + currentCrossPos + itemCrossOffset;
                    currentMainPos += itemSize.x + gap;
                }
                else {
                    finalX = padding.x + currentCrossPos + itemCrossOffset;
                    finalY = padding.y + currentMainPos;
                    currentMainPos += itemSize.y + gap;
                }

                item->setPosition(position + sf::Vector2f(finalX, finalY));
                
                float itemBottom = finalY + itemSize.y + padding.y;
                if (itemBottom > maxContentHeight) maxContentHeight = itemBottom;
            }
            currentCrossPos += line.crossSize + gap;
        }
        
        // Виджеты, которые не участвуют в расстановке, принудительно ставятся в начало контейнера
        for (auto& child : children) {
            if (child->isVisible() && !child->getFollowsLayout()) {
                child->setPosition(position);
            }
        }
    }

    // Передача системных событий дочерним виджетам с учетом прокрутки и вьюпорта
    void Container::handleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& uiView) {
        if (!enabled || !visible) return;

        sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos, uiView);
        bool mouseInside = getGlobalBounds().contains(mousePos);

        // ОБРАБОТКА ПРОКРУТКИ (Колесико мыши + Сенсорное перетаскивание)
        if (scrollEnabled) {
            // Колесико мыши
            if (mouseInside) {
                if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
                    if (scroll->wheel == sf::Mouse::Wheel::Vertical) {
                        scrollOffset -= scroll->delta * 40.f;
                    }
                }
            }

            // Сенсорное перетаскивание (Android)
            if (const auto* touch = event.getIf<sf::Event::TouchBegan>()) {
                sf::Vector2f touchPos = window.mapPixelToCoords(touch->position, uiView);
                if (getGlobalBounds().contains(touchPos)) {
                    isDragging = true;
                    lastTouchPos = touch->position;
                }
            }
            else if (const auto* tMoved = event.getIf<sf::Event::TouchMoved>()) {
                if (isDragging) {
                    float deltaY = static_cast<float>(lastTouchPos.y - tMoved->position.y);
                    scrollOffset += deltaY;
                    lastTouchPos = tMoved->position;
                }
            }
            else if (event.is<sf::Event::TouchEnded>()) {
                isDragging = false;
            }

            // Ограничение скролла
            float maxScroll = std::max(0.f, maxContentHeight - size.y);
            if (scrollOffset < 0.f) scrollOffset = 0.f;
            if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        }

        // Если скролл включен, создаем скорректированный вид с вьюпортом для детей
        if (scrollEnabled) {
            sf::View adjustedView(sf::FloatRect({ position.x, position.y + scrollOffset }, size));
            
            sf::Vector2u winSize = window.getSize();
            sf::Vector2i topLeft = window.mapCoordsToPixel(position, uiView);
            sf::Vector2i bottomRight = window.mapCoordsToPixel(position + size, uiView);

            sf::FloatRect viewport;
            viewport.position.x = static_cast<float>(topLeft.x) / static_cast<float>(winSize.x);
            viewport.position.y = static_cast<float>(topLeft.y) / static_cast<float>(winSize.y);
            viewport.size.x = static_cast<float>(bottomRight.x - topLeft.x) / static_cast<float>(winSize.x);
            viewport.size.y = static_cast<float>(bottomRight.y - topLeft.y) / static_cast<float>(winSize.y);
            
            adjustedView.setViewport(viewport);

            for (auto it = children.rbegin(); it != children.rend(); ++it) {
                if ((*it)->isEnabled()) {
                    (*it)->handleEvent(event, window, adjustedView);
                }
            }
        } else {
            for (auto it = children.rbegin(); it != children.rend(); ++it) {
                if ((*it)->isEnabled()) {
                    (*it)->handleEvent(event, window, uiView);
                }
            }
        }
    }

    // Отрисовка контейнера
    void Container::render(sf::RenderWindow& window) const {
        if (!visible) return;

        if (drawBackground) window.draw(background);
        if (drawOutline) window.draw(outline);

        if (scrollEnabled) {
            sf::View currentView = window.getView();
            sf::Vector2u winSize = window.getSize();

            sf::Vector2i topLeft = window.mapCoordsToPixel(position, currentView);
            sf::Vector2i bottomRight = window.mapCoordsToPixel(position + size, currentView);

            sf::FloatRect viewport;
            viewport.position.x = static_cast<float>(topLeft.x) / static_cast<float>(winSize.x);
            viewport.position.y = static_cast<float>(topLeft.y) / static_cast<float>(winSize.y);
            viewport.size.x = static_cast<float>(bottomRight.x - topLeft.x) / static_cast<float>(winSize.x);
            viewport.size.y = static_cast<float>(bottomRight.y - topLeft.y) / static_cast<float>(winSize.y);

            if (viewport.size.x <= 0.f || viewport.size.y <= 0.f) return;

            sf::View scrolledView(sf::FloatRect({ position.x, position.y + scrollOffset }, size));
            scrolledView.setViewport(viewport);

            window.setView(scrolledView);
            for (const auto& child : children) {
                if (child->isVisible()) child->render(window);
            }
            window.setView(currentView);
        }
        else {
            for (const auto& child : children) {
                if (child->isVisible()) child->render(window);
            }
        }
    }

    // Получение границ контейнера
    sf::FloatRect Container::getGlobalBounds() const {
        return { position, size };
    }

}
