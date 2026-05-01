# Отчет по оптимизации кода Tower Defence

## Обзор проекта

Проект представляет собой игру в жанр Tower Defence, реализованную на C++ с использованием библиотеки SFML. Код хорошо структурирован и разделен на логические модули: игровая механика, интерфейс, управление ресурсами, улучшениями и сохранениями.

## Основные проблемы и рекомендации по оптимизации

### 1. Повторяющиеся блоки создания ресурсов

**Проблема:** В файле `main.cpp` в функции `loadResources()` наблюдается значительное дублирование кода при загрузке ресурсов.

**Пример проблемы:**
```cpp
// ШРИФТ
ResourceManager::loadFont("main", assetsPath + "fonts/web_ibm_mda.ttf");

// ИКОНКИ
ResourceManager::load("icon-coins", assetsPath + "icons/coins.png");
ResourceManager::load("icon-heart", assetsPath + "icons/heart.png");
// ... множество аналогичных строк
```

**Влияние:** Увеличивает размер кода, затрудняет поддержку, повышает вероятность ошибок при добавлении новых ресурсов.

**Решение:** Создать функцию-пакет для групповой загрузки ресурсов:
```cpp
// В ResourceManager.hpp
static void loadResourcesFromList(const std::string& assetsPath, 
                                const std::vector<std::pair<std::string, std::string>>& resources);

// В ResourceManager.cpp
void ResourceManager::loadResourcesFromList(const std::string& assetsPath, 
                                          const std::vector<std::pair<std::string, std::string>>& resources) {
    for (const auto& [name, path] : resources) {
        load(name, assetsPath + path);
    }
}

// Использование в main.cpp
std::vector<std::pair<std::string, std::string>> iconResources = {
    {"icon-coins", "icons/coins.png"},
    {"icon-heart", "icons/heart.png"},
    // ... остальные иконки
};

std::vector<std::pair<std::string, std::string>> buttonResources = {
    {"button", "sprites/button.png"},
    {"button-hover", "sprites/button-hover.png"},
    // ... остальные кнопки
};

ResourceManager::loadFont("main", assetsPath + "fonts/web_ibm_mda.ttf");
ResourceManager::loadResourcesFromList(assetsPath, iconResources);
ResourceManager::loadResourcesFromList(assetsPath, buttonResources, false); // с параметром smooth
```

### 2. Дублирование логики обработки оверлеев

**Проблема:** В файле `Game.cpp` в функции `initOverlays()` наблюдается значительное дублирование при создании паузы и финального экрана.

**Пример проблемы:**
Практически идентичный код для создания кнопок оверлеев:
```cpp
// Внутри лямбда-функции createOverlayButton
auto btn = std::make_unique<UI::Button>(font, label, btnSize);
btn->setBackgroundTextures(
    &ResourceManager::get("button"),
    &ResourceManager::get("button-hover"),
    &ResourceManager::get("button-active"),
    &ResourceManager::get("button-disabled"),
    32.0f
);
btn->setTextSize(20);
btn->setCallback(std::move(onClick));
return btn;
```

**Влияние:** Нарушение принципа DRY (Don't Repeat Yourself), усложнение поддержки.

**Решение:** Вынести создание стилизованной кнопки в отдельную функцию:
```cpp
// В Game.cpp
static std::unique_ptr<UI::Button> createStyledButton(
    const sf::Font& font, 
    const std::string& label, 
    const sf::Vector2f& size,
    std::function<void()> onClick) {
    
    auto btn = std::make_unique<UI::Button>(font, label, size);
    btn->setBackgroundTextures(
        &ResourceManager::get("button"),
        &ResourceManager::get("button-hover"),
        &ResourceManager::get("button-active"),
        &ResourceManager::get("button-disabled"),
        32.0f
    );
    btn->setTextSize(20);
    btn->setCallback(std::move(onClick));
    return btn;
}

// Использование
pNav->addChild(createStyledButton(font, "В МЕНЮ", btnSize, [this]() { endReason = GameEndReason::ReturnToMenu; }));
```

### 3. Повторяющиеся вычисления в циклах

**Проблема:** В файле `Enemy.cpp` в функции `getVelocity()` происходит повторное вычисление длины вектора и нормализация.

**Пример проблемы:**
```cpp
sf::Vector2f Enemy::getVelocity() const {
    // ...
    sf::Vector2f dir = targetPos - pos;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    // ...
    if (len < 0.1f) return { 0.f, 0.f };
    return (dir / len) * (float)speed;
}
```

**Влияние:** Лишние вычисления квадратного корня и деления каждый кадр для каждого врага.

**Решение:** Кэшировать нормализованный вектор направления и обновлять его только при смене цели:
```cpp
// В Enemy.hpp добавить поле:
sf::Vector2f direction; // текущее направление движения

// В Enemy.cpp
void Enemy::update(float deltaTime) {
    // ...
    if (distSq < 4.f) {
        pathIndex++;
        // Пересчитываем направление только при смене точки пути
        if (pathIndex < (int)path->size()) {
            sf::Vector2f target = sf::Vector2f((*path)[pathIndex] * 64);
            direction = Math::normalize(target - pos);
        }
    } else {
        pos += direction * (float)speed * deltaTime;
    }
}

sf::Vector2f Enemy::getVelocity() const {
    if (path->empty() || pathIndex >= (int)path->size()) 
        return { 0.f, 0.f };
    return direction * (float)speed;
}
```

### 4. Неоптимальная работа со строками

**Проблема:** В множестве мест используется конкатенация строк через оператор `+` в циклах, что приводит к множеству временных объектов.

**Пример проблемы:** В `Tower.cpp` конструктор:
```cpp
textureBase = &ResourceManager::get("tower-" + typeSlug + "-base");
textureTower = &ResourceManager::get("tower-" + typeSlug + "-turret");
```

**Влияние:** Создание временных строк при каждой операции конкатенации, особенно критично в конструкторах, вызываемых часто.

**Решение:** Использовать `std::string::reserve()` или предвычислять идентификаторы:
```cpp
// В Tower.cpp
std::string baseId = "tower-" + typeSlug + "-base";
std::string turretId = "tower-" + typeSlug + "-turret";
textureBase = &ResourceManager::get(baseId);
textureTower = &ResourceManager::get(turretId);
```

Или лучше - хранить префикс как поле класса:
```cpp
// В Tower.hpp
std::string towerPrefix; // например, "tower-basic"

// В конструкторе
towerPrefix = "tower-" + typeSlug;
textureBase = &ResourceManager::get(towerPrefix + "-base");
textureTower = &ResourceManager::get(towerPrefix + "-turret");
```

### 5. Дублирование логики получения стоимости продажи

**Проблема:** В `Game.cpp` логика расчёта стоимости продажи башни дублируется в двух местах:
1. В `handleEvents()` при обработке продажи (строки 240-241)
2. В `render()` при отображении подсказки (строки 412-413)

**Пример проблемы:**
```cpp
// В handleEvents()
float refundPercent = (waveSystem.getState() == WaveState::Idle) ? 1.0f : 0.65f;
money += static_cast<int>(it->getTotalValue() * refundPercent);

// В render()
float refundPercent = (waveSystem.getState() == WaveState::Idle) ? 1.0f : 0.65f;
int sellPrice = (int)(it->getTotalValue() * refundPercent);
```

**Влияние:** Нарушение принципа единой ответственности, риск расхождения логики при изменении правил.

**Решение:** Вынести расчёт стоимости продажи в отдельную функцию:
```cpp
// В Game.cpp
static int calculateSellPrice(const Tower& tower, WaveState waveState) {
    float refundPercent = (waveState == WaveState::Idle) ? 1.0f : 0.65f;
    return static_cast<int>(tower.getTotalValue() * refundPercent);
}

// Использование
int sellPrice = calculateSellPrice(*it, waveSystem.getState());
```

### 6. Неоптимальная работа с контейнерами

**Проблема:** В `Game.cpp` в функции `update()` используется `std::remove_if` с лямбда-функцией для очистки мёртвых объектов, что создает дополнительные копии.

**Пример проблемы:**
```cpp
projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), 
    [](const Projectile& p) { return !p.isAlive(); }), projectiles.end());
```

**Влияние:** Создание временных объектов при каждом вызове лямбда-функции.

**Решение:** Использовать более эффективный алгоритм или предвычислять флаги жизни:
```cpp
// Альтернативный подход - ручная итерация
auto it = projectiles.begin();
while (it != projectiles.end()) {
    if (!it->isAlive()) {
        it = projectiles.erase(it);
    } else {
        ++it;
    }
}

// Или добавить флаг "мечен на удаление" в объект
```

### 7. Дублирование констант цветов и размеров

**Проблема:** В множестве файлов используются "магические числа" для размеров, цветов и других параметров.

**Пример проблемы:** В `Game.cpp`:
```cpp
sf::Vector2f btnSize(220.f, 60.f);
btn->setTextSize(20);
// ...
auto pTitle = std::make_unique<UI::Text>(font, "ПАУЗА", 96, sf::Vector2f(winSize.x * 0.9f, 100.f));
```

**Влияние:** Сложность поддержки при необходимости изменить дизайн интерфейса.

**Решение:** Создать файл с константами UI:
```cpp
// В UIConstants.hpp
namespace UIConstants {
    const sf::Vector2f BUTTON_SIZE{220.f, 60.f};
    const unsigned int BUTTON_TEXT_SIZE = 20;
    const sf::Vector2f PAUSE_TITLE_SIZE{0.9f, 100.f};
    const unsigned int PAUSE_TITLE_FONT_SIZE = 96;
    // ... другие константы
}

// Использование
sf::Vector2f btnSize = UIConstants::BUTTON_SIZE;
btn->setTextSize(UIConstants::BUTTON_TEXT_SIZE);
```

## Общие рекомендации

1. **Внедрить систему логгирования производительности** для выявления узких мест в реальном времени
2. **Рассмотреть использование object pooling** для часто создаваемых и уничтожаемых объектов (снаряды, частицы)
3. **Оптимизировать проверки столкновений** через пространственное partitioning (например, grid-based систему)
4. **Внедрить систему событий** для уменьшения сильной связанности между компонентами
5. **Рассмотреть использование consteval/constexpr** для вычисления констант во время компиляции

## Заключение

Код проекта Tower Defence имеет хорошую архитектуру и следует принципам объектно-ориентированного программирования. Основные возможности оптимизации связаны с устранением дублирования кода, улучшением работы со строками и контейнерами, а также внедрением систем констант и вспомогательных функций для уменьшения копипаста.

Реализация предложенных улучшений сделает код более поддерживаемым, уменьшит вероятность ошибок и потенциально улучшит производительность за счет сокращения лишних вычислений и временных объектов.