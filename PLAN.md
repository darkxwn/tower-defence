Ниже представлен финальный, максимально подробный и структурированный план разработки (PLAN.md), объединяющий все предыдущие правки, утвержденный баланс и требуемую архитектуру.

---

# PLAN.md — Глобальное обновление: Бесконечный режим, Баланс и Прогрессия

## 1. Парсеры и Балансировка Конфигураций
**Описание:** Очистка парсеров от хардкода, внедрение игнорирования комментариев, добавление новых параметров статов врагов и настройка баланса для создания дефицита ресурсов.
**Затронутые файлы:** `GameData.hpp`, `GameData.cpp`, `Map.hpp`, `Map.cpp`.

**Способы решения (Чтение списков в `.map`):**
*   *Вариант А:* Парсинг через `std::string::find` и обрезку пробелов вручную.
*   *Вариант Б:* Использование `std::stringstream`, который автоматически обрабатывает любое количество пробелов и извлекает данные.
*   **Выбранный вариант:** Вариант Б, так как он безопаснее и код получается чище.

**Изменения в коде:**
*   **Игнорирование комментариев:** Во все циклы `std::getline` (для конфигов и карт) добавляется проверка: `if (line.empty() || line[0] == '#' || line[0] == '\r') continue;`.
*   **Новые поля в `GameData`:** В `EnemyStats` добавляются `int armor`, `int points`, `float spawnInterval`.
*   **Новые поля в `Map`:** 
    *   Переименование стартового капитала `money` в `coins` (`startCoins`).
    *   Вектор `std::vector<std::string> allowedEnemies` (парсится из `enemies=basic fast`).
    *   Вектор `std::vector<int> starThresholds` (парсится из `stars=10 25 50`).
*   **Изменения в GameData.hpp:**
    ```hpp
    // Характеристики врага
    struct EnemyStats {
        int health;          // количество жизней
        int speed;           // скорость передвижения
        int damage;          // урон по базе
        int reward;          // награда за уничтожение (coins)
        int points;          // количество очков за убийство
        float spawnInterval; // интервал спавна
        int armor;           // броня (снижает получаемый урон)
    };
    ```
    Изменится порядок сканирования в `.cpp` файле

**Пример кода (Парсинг списков в `Map.cpp`):**
```cpp
// ... внутри цикла while (std::getline(file, line))
if (line.rfind("enemies=", 0) == 0) {
    std::stringstream ss(line.substr(8));
    std::string type;
    while (ss >> type) allowedEnemies.push_back(type);
} else if (line.rfind("stars=", 0) == 0) {
    std::stringstream ss(line.substr(6));
    int val;
    while (ss >> val) starThresholds.push_back(val);
} else if (line.rfind("coins=", 0) == 0) {
    startCoins = std::stoi(line.substr(6));
}
```

**Отбалансированные конфиги:**
```cfg
# enemies.cfg (Строгий порядок параметров)
basic health=30 armor=0 speed=50 damage=1 reward=3 points=10 spawnInterval=1.0
fast health=15 armor=0 speed=100 damage=1 reward=2 points=15 spawnInterval=0.5
strong health=120 armor=4 speed=35 damage=3 reward=8 points=40 spawnInterval=1.5
```
```cfg
# towers.cfg (Цены увеличены под reward 2-8, урон пересчитан под броню)
basic damage=10 firerate=1.0 range=128 cost=80 splash=0 rank=0 level=0 costRank=250 costDamage=60 costFirerate=80 costRange=80 costLevel=200
double damage=5 firerate=3.0 range=160 cost=200 splash=0 rank=0 level=0 costRank=500 costDamage=100 costFirerate=150 costRange=120 costLevel=400
cannon damage=30 firerate=0.5 range=128 cost=350 splash=96 rank=0 level=0 costRank=800 costDamage=180 costFirerate=250 costRange=200 costLevel=700
sniper damage=80 firerate=0.25 range=320 cost=600 splash=0 rank=0 level=0 costRank=1200 costDamage=300 costFirerate=400 costRange=350 costLevel=1000
```

---

## 2. Механика Брони и Система Очков
**Описание:** Добавление показателя брони, который снижает получаемый башнями урон, и очков, которые начисляются за уничтожение врагов.
**Затронутые файлы:** `Enemy.hpp`, `Enemy.cpp`.

**Способы решения (Расчет брони):**
*   *Вариант А:* Процентное снижение (например, броня 10 = -10% урона).
*   *Вариант Б:* Прямое вычитание (Flat reduction).
*   **Выбранный вариант:** Вариант Б. Это заставляет игрока использовать Снайперов против бронированных целей (Двойная башня с уроном 5 не пробьет броню 4, нанося лишь 1 урон).

**Изменения в коде:**
*   Добавление полей `int armor; int points;` в класс `Enemy` (инициализируются из `GameData` в конструкторе).
*   Изменение метода получения урона:
```cpp
void Enemy::takeDamage(int damage) {
    // Минимальный урон всегда 1, чтобы башни не становились полностью бесполезными
    int finalDamage = std::max(1, damage - armor); 
    health -= finalDamage;
    if (health <= 0) alive = false;
}
```

---

## 3. Процедурные Бесконечные Волны
**Описание:** Удаление хардкода. Динамическая генерация, где 1 волна = 1 тип врагов. Тип фиксируется на несколько волн.
**Затронутые файлы:** `WaveSystem.hpp`, `WaveSystem.cpp`.

**Способы решения (Расчет количества врагов):**
*   *Вариант А:* Линейный рост `Count = Base + Wave * X`.
*   *Вариант Б:* Ступенчатый рост с отклонением `Count = Base + (Wave/3)*5 + Random`.
*   **Выбранный вариант:** Вариант Б. Создает динамику: три волны количество примерно одинаковое, на четвертую резкий скачок ("волна-испытание").

**Изменения в коде:**
*   Удаляется список волн `std::vector<Wave> waves`.
*   Добавляются переменные: `std::string currentEnemyType`, `int wavesUntilTypeChange`.
*   Метод `startWave()`:
```cpp
currentWave++;
// 1. Смена типа врага
if (wavesUntilTypeChange <= 0) {
    int idx = Math::Random::getInt(0, mapAllowedEnemies.size() - 1);
    currentEnemyType = mapAllowedEnemies[idx];
    wavesUntilTypeChange = Math::Random::getInt(2, 4); // Держим тип от 2 до 4 волн
}
wavesUntilTypeChange--;

// 2. Расчет количества
totalInWave = (8 + (currentWave / 3) * 5) + Math::Random::getInt(-3, 3);
spawnedCount = 0;
```
*   Метод `update()` (Создание врага):
```cpp
// 3. Экспоненциальное масштабирование HP (броня не масштабируется!)
int scaledHp = static_cast<int>(baseStats.health * std::pow(1.06f, currentWave - 1));
enemies.push_back(std::make_unique<Enemy>(currentEnemyType, scaledHp, baseStats.speed, baseStats.reward, path));
```

---

## 4. Экономика, Мета-прогрессия и Сохранения
**Описание:** Разделение валют (`coins` для боя, `money` для улучшений), сохранение рекордов и добавление глобальных бонусов. Все ключи JSON строго в `camelCase`.
**Затронутые файлы:** `SaveManager.hpp`, `SaveManager.cpp`, `Game.hpp`, `Game.cpp`.

**Изменения в коде:**
*   **SaveManager:** 
    *   В `LevelProgress` добавить: `int stars`, `int bestScore`, `int maxWave`.
    *   Новые глобальные переменные: `globalCoinsLvl`, `globalMoneyLvl`, `globalBaseHpLvl`.
*   **Интеграция в Game (Старт игры):**
```cpp
// Применяем мета-бонусы при загрузке уровня
coins = map.getStartCoins() + (saveManager.getGlobalCoinsLvl() * 15);
base = Base(map.getBasePos(), 20 + saveManager.getGlobalBaseHpLvl());
// Устанавливаем множитель меты
saveManager.setMoneyMultiplier(1.0f + (saveManager.getGlobalMoneyLvl() * 0.1f));
```
*   **Интеграция в Game (Убийство врага):**
```cpp
if (e->isKilled()) {
    currentScore += e->getPoints(); // Игровые очки
    coins += e->getReward();        // Внутриигровая валюта
    // Выпадение мета-валюты (сохраняется ваша оригинальная логика)
    accumulatedGlobalMoney += upgradeManager.getRandomMoney(saveManager.getMoneyMultiplier()); 
}
```
*   **Запись рекордов (Конец сессии):**
    В `Game::cleanup` сверяем `currentWave` и `currentScore` с сохраненными в `SaveManager`. Обновляем максимум. Рассчитываем звезды, проверяя достигнутую волну по `map.getStarThresholds()`.

---

## 5. Внутриигровой Интерфейс (HUD)
**Описание:** Визуализация текущего счета во время боя.
**Затронутые файлы:** `HUD.hpp`, `HUD.cpp`.

**Изменения в коде:**
*   Добавление поля `UI::Text scoreText` в класс `HUD`.
*   В конструкторе `HUD` инициализация: `scoreText = UI::Text(*mainFont, "Очки: 0", 22);`.
*   В методе `HUD::render` добавление передачи очков и обновление текста:
```cpp
void HUD::render(sf::RenderWindow& window, int money, int lives, int wave, WaveState state, int currentScore) {
    // ...
    scoreText.setText("Очки: " + std::to_string(currentScore));
    // Выравнивание строго под waveText
    scoreText.setPosition({ cx, waveText.getPosition().y + 30.f });
    window.draw(scoreText);
}
```

---

## 6. Редизайн Главного Меню (Карточки и Улучшения)
**Описание:** Обновление карточек уровней под новую структуру рекордов и создание двухуровневого скролл-меню улучшений.
**Затронутые файлы:** `Menu.cpp`.

**Способы решения (Меню улучшений):**
*   *Вариант А:* Две разные вкладки (кнопки переключения).
*   *Вариант Б:* Единый вертикальный контейнер со скроллом и заголовками блоков.
*   **Выбранный вариант:** Вариант Б. Быстрее в навигации, идеально для мобильных.

**Изменения в коде:**
*   **Карточки уровней (строгий порядок сверху вниз):**
```cpp
// 1. Номер
card->addChild(std::make_unique<UI::Text>(font, "Уровень " + std::to_string(lvl.index + 1), 20));
// 2. Название
card->addChild(std::make_unique<UI::Text>(font, lvl.name, 26));
// 3. Рекорд
std::string recordStr = "Рекорд: ур. " + std::to_string(maxWave) + " | " + std::to_string(bestScore);
card->addChild(std::make_unique<UI::Text>(font, recordStr, 16));
// 4. Звезды
auto starsRow = std::make_unique<UI::Container>(...);
starsRow->setDirection(UI::Container::Direction::Row);
int savedStars = saveManager.getStars(lvl.id);
for(int i = 0; i < 3; ++i) {
    const auto& tex = (i < savedStars) ? ResourceManager::get("star-filled") : ResourceManager::get("star-empty");
    starsRow->addChild(std::make_unique<UI::Image>(tex, sf::Vector2f(24.f, 24.f)));
}
card->addChild(std::move(starsRow));
```
*   **Меню улучшений:** 
    1.  `UI::Text` "МОДЕРНИЗАЦИЯ ТУРЕЛЕЙ".
    2.  Существующий `Container` с сеткой прокачки башен.
    3.  `UI::Text` "СТРАТЕГИЧЕСКИЙ ОТДЕЛ".
    4.  Вертикальный `Container` с тремя строками-улучшениями (Начальный капитал, Доходность, Жизни базы), привязанными к мета-валюте `money`.

---

## 7. Защита сохранений (Anti-Tamper Hash)
**Описание:** Базовая защита файла `progress.json` от накрутки значений.
**Затронутые файлы:** `SaveManager.cpp`.

**Способы решения:**
*   *Вариант А:* Бинарное шифрование.
*   *Вариант Б:* `std::hash` валидация.
*   **Выбранный вариант:** Вариант Б. Не усложняет чтение файла, но делает изменение значений вручную невозможным без пересчета хэша.

**Изменения в коде:**
```cpp
// В SaveManager::save():
std::string dataDump = j.dump();
size_t hashChecksum = std::hash<std::string>{}(dataDump + "GyurzaSecretSalt_2025");
j["hashChecksum"] = hashChecksum; // Пишем хэш в сам json
file << j.dump(4);

// В SaveManager::load():
size_t savedHash = j.value("hashChecksum", 0ULL);
j.erase("hashChecksum"); // Удаляем перед сверкой
std::string dataDump = j.dump();
size_t calculatedHash = std::hash<std::string>{}(dataDump + "GyurzaSecretSalt_2025");

if (savedHash != 0ULL && savedHash != calculatedHash) {
    Logger::error("Anti-Tamper: Файл сохранения изменен вручную! Сброс прогресса.");
    setDefaults();
    return;
}
```