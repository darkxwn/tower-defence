# PLAN.md — Планирование задач

---

## Текущая задача: Контейнер в стиле CSS Flexbox

### Описание
Переработать `UI::Container` на основе CSS Flexbox-механизма. Вместо фиксированных режимов (`Layout::Vertical/Horizontal/Grid`) использовать набор независимых свойств: направление оси, выравнивание по главной оси, выравнивание по поперечной оси, перенос, отступ. Это повторяет поведение CSS: `display: flex`, `flex-direction`, `justify-content`, `align-items`, `flex-wrap`, `gap`.

Визуальный прототип в `container-context/menu-mockup.html` показывает структуру меню и соответствие CSS-свойств C++-методам.

---

## Способы решения

### 1. Независимые flexbox-свойства (выбрано)

**Описание:** Заменить enum `Layout` на набор независимых enum-свойств и один метод расчёта `recalculateLayout()`.

```cpp
enum class Direction { Row, Column };          // направление оси
enum class ContentAlign { Start, Center, End }; // выравнивание по главной оси
enum class ItemAlign { Start, Center, End };    // выравнивание по поперечной оси
```

**Соответствие CSS → C++:**

| CSS | C++ метод |
|-----|-----------|
| `flex-direction: row` | `setDirection(Row)` |
| `flex-direction: column` | `setDirection(Column)` |
| `justify-content: flex-start` | `setContentAlign(Start)` |
| `justify-content: center` | `setContentAlign(Center)` |
| `justify-content: flex-end` | `setContentAlign(End)` |
| `align-items: flex-start` | `setItemAlign(Start)` |
| `align-items: center` | `setItemAlign(Center)` |
| `align-items: flex-end` | `setItemAlign(End)` |
| `flex-wrap: wrap` | `setWrap(true)` |
| `gap: 10px` | `setGap(10.f)` |

**Пример использования (главное меню):**
```cpp
mainMenu->setDirection(Container::Column);
mainMenu->setContentAlign(Container::Center);
mainMenu->setItemAlign(Container::Center);
mainMenu->setGap(16.f);
mainMenu->setPadding({ 20.f, 20.f });
```

**Плюсы:**
- Один универсальный механизм вместо трёх фиксированных режимов
- Совпадает с CSS-логикой (прототип в HTML сразу переносится в C++)
- Независимые свойства комбинируются произвольно
- `wrap` заменяет `Grid` — элементы сами переносятся

**Минусы:**
- Полная переработка `recalculateLayout()`
- Удаление старых методов: `setLayout()`, `setSpacing()`, `setCenterContent()`, `setGridColumnCount()`, `setGridCellSize()`, `setGridSpacing()`
- Требуется обновление всех вызовов в `Menu.cpp`

**Выбор:** Вариант 1. Flexbox-подход — чище, гибче, соответствует прототипу в `container-context/`.

---

## Этапы разработки

1. **Изменить `Container.hpp`** — удалить `Layout`, добавить `Direction`, `ContentAlign`, `ItemAlign`, поля и сеттеры
2. **Изменить `Container.cpp`** — переписать `recalculateLayout()` под flexbox-логику с учётом wrap
3. **Обновить `Menu.cpp`** — заменить вызовы старых методов на новые
4. **Проверить соответствие** `.cpp` и `.hpp`
5. **Собрать и проверить** все экраны меню

---

## Завершённые задачи

### Контейнер — внедрение в Menu
- `std::vector<UI::Button>` заменены на `std::unique_ptr<UI::Container>` для 4 экранов
- Контейнеры создаются один раз в конструкторе, не пересоздаются при ресайзе
- `updateViewSizes` — `setSize` + `setPosition` (вызывает `recalculateLayout`)
- `handleEvents` — `container->handleEvent` вместо цикла по вектору
- `render` — `container->render` вместо цикла
- Добавлена обводка (`setDrawOutline`) для отладки
- Добавлено `setCenterContent` для центрирования содержимого
- Файлы затронуты: `Container.hpp`, `Container.cpp`, `Menu.hpp`, `Menu.cpp`, `main.cpp`
