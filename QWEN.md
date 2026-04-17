# QWEN.md — Codename: Tower Defence

## Обзор проекта

**Codename: Tower Defence** — классическая игра в жанре «Башенная защита» с сеточной картой и волнами противников. Проект разрабатывается в рамках курсовой работы студентов 2-го курса факультета ИСТ ДонНТУ.

- **Язык:** C++20
- **Графическая библиотека:** SFML 3.0.2
- **Сборка:** CMake 3.15+
- **IDE:** Visual Studio 2022 (рекомендуется)
- **Платформы:** Windows, Linux, macOS, Android

## Структура проекта

```
tower-defence/
├── main.cpp                    # Точка входа, главный цикл (Menu <-> Game)
├── CMakeLists.txt              # Основная конфигурация CMake
├── CMakePresets.json           # Пресеты сборки (Windows x64 Debug, Android arm64)
│
├── includes/                   # Заголовочные файлы (.hpp)
│   ├── ui/                     # UI-компоненты (Button, Text)
│   ├── utils/                  # Утилиты (Logger, FileReader)
│   ├── Game.hpp                # Главный игровой класс
│   ├── Menu.hpp                # Система меню
│   ├── Map.hpp                 # Карта уровня
│   ├── Tower.hpp               # Башни
│   ├── Enemy.hpp               # Враги
│   ├── Projectile.hpp          # Снаряды
│   ├── WaveSystem.hpp          # Система волн
│   ├── HUD.hpp                 # Интерфейс (HUD)
│   ├── Base.hpp                # База (цель врагов)
│   ├── GameData.hpp            # Игровые данные
│   ├── ResourceManager.hpp     # Менеджер ресурсов
│   ├── SettingsManager.hpp     # Менеджер настроек
│   ├── Tile.hpp                # Типы тайлов
│   └── Colors.hpp              # Цветовые константы
│
├── src/                        # Исходные файлы (.cpp)
│   ├── ui/                     # Реализации UI-компонентов
│   └── utils/                  # Реализации утилит
│
├── assets/                     # Игровые ресурсы
│   ├── fonts/                  # Шрифты
│   ├── icons/                  # Иконки интерфейса
│   ├── sounds/                 # Звуки
│   └── sprites/                # Спрайты (тайлы, башни, враги, снаряды)
│
├── data/                       # Игровые данные
│   ├── config/                 # Конфигурации
│   │   ├── towers.cfg          # Параметры башен (урон, скорострельность, дальность, цена, сплэш)
│   │   └── enemies.cfg         # Параметры врагов (здоровье, скорость, урон, награда)
│   ├── levels/                 # Файлы уровней (.map) — 9 уровней
│   ├── game/                   # Сохранения игры
│   └── localization/           # Локализация
│
├── android/                    # Android-проект (Gradle)
├── macos/                      # macOS-специфичные файлы (Info.plist.in)
└── out/                        # Артефакты сборки (игнорируется в .gitignore)
```

## Типы тайлов (`TileType`)

| ID | Тип        | Описание                               |
|----|------------|----------------------------------------|
| 0  | Empty      | Пустой тайл (фон)                      |
| 1  | Portal     | Портал — точка появления врагов         |
| 2  | Road       | Дорога — путь врагов к базе            |
| 3  | Base       | База — цель врагов (защищать)          |
| 4  | Platform   | Платформа — место для постройки башни  |

## Башни (из `towers.cfg`)

| Тип      | Урон | Скорострельность | Дальность | Цена | Сплэш |
|----------|------|------------------|-----------|------|-------|
| basic    | 35   | 1.0              | 192       | 75   | 0     |
| cannon   | 80   | 0.7              | 160       | 150  | 64    |
| double   | 20   | 2.0              | 128       | 100  | 0     |
| sniper   | 150  | 0.4              | 320       | 200  | 0     |

## Враги (из `enemies.cfg`)

| Тип      | Здоровье | Скорость | Урон | Награда |
|----------|----------|----------|------|---------|
| basic    | 125      | 48       | 1    | 35      |
| fast     | 75       | 128      | 1    | 40      |
| strong   | 300      | 32       | 3    | 50      |

## Формат файла уровня (`.map`)

```
width=10           # Ширина карты
height=6           # Высота карты
money=250          # Стартовые деньги
name=Начало        # Название уровня
tiles=             # Сетка тайлов (числа через пробел)
0 0 0 0 0 0 0 0 0 0
0 1 2 2 0 0 0 0 0 0
...
waves=             # Волны (формат: тип_врага:количество)
basic:4
basic:4
...
```

## Сборка и запуск

### Windows (Visual Studio 2022)

1. Откройте репозиторий как CMake-проект в Visual Studio 2022
2. Дождитесь конфигурации CMake
3. Выберите target `tower-defence.exe`
4. Соберите и запустите

**Консольная сборка (Ninja):**
```bash
cmake --preset windows-x64-debug
cmake --build out/build/windows-x64-debug
```

### Linux

```bash
# Установка зависимостей (Ubuntu/Debian)
sudo apt update
sudo apt install -y build-essential cmake git \
  libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
  libgl1-mesa-dev libglu1-mesa-dev libfreetype-dev \
  libopenal-dev libogg-dev libvorbis-dev libflac-dev \
  libwayland-dev libxkbcommon-dev wayland-protocols

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./tower-defence
```

### Android

Требуется NDK и переменная окружения `ANDROID_NDK_HOME`:
```bash
cmake --preset android-arm64
cmake --build out/build/android-arm64
```
Сборка APK осуществляется через Gradle из папки `android/`.

### macOS

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
open ./tower-defence.app
```

## Архитектура

### Главный цикл (`main.cpp`)

```
while (window.isOpen()) {
    ├── Меню (пока уровень не выбран)
    │   ├── Главное меню
    │   ├── Выбор уровня
    │   ├── Настройки
    │   └── Улучшения (заглушка)
    │
    └── Игра (выбранный уровень)
        ├── Playing → Restart (перезапуск)
        ├── Playing → Win → Меню (баннер победы)
        ├── Playing → Lose → Меню (баннер поражения)
        └── Playing → ReturnToMenu
}
```

### Основные классы

| Класс            | Назначение                                          |
|------------------|-----------------------------------------------------|
| `Game`           | Главный игровой класс: обновление, отрисовка, ввод |
| `Menu`           | Система меню с несколькими экранами                 |
| `Map`            | Загрузка и отрисовка карты уровня                     |
| `Tower`          | Логика башен (поиск цели, стрельба)                 |
| `Enemy`          | Логика врагов (движение по пути, атака базы)        |
| `WaveSystem`     | Управление волнами врагов                            |
| `Projectile`     | Летящие снаряды                                      |
| `Base`           | База игрока (здоровье)                              |
| `HUD`            | Интерфейс (деньги, жизни, выбор башен)              |
| `ResourceManager`| Загрузка и кэширование ресурсов                      |
| `SettingsManager`| Настройки (звук, фуллскрин, масштаб UI)             |
| `GameData`       | Глобальные игровые данные и прогресс                 |

## Конвенции

- **Язык кода:** английский (имена переменных, комментарии)
- **Комментарии:** разделы отделяются блоками `///////////////////////////////////////////////////////////////////////////`
- **Стандарт C++:** C++20
- **Статическая линковка:** SFML собирается статически (`BUILD_SHARED_LIBS OFF`)
- **Ресурсы:** копируются в выходную директорию автоматически через CMake
- **Платформенные макросы:** `__ANDROID__`, `__APPLE__`, `WIN32` для условной компиляции

## Ключевые команды

| Действие                          | Команда                                                       |
|-----------------------------------|---------------------------------------------------------------|
| Конфигурация (Windows Debug)      | `cmake --preset windows-x64-debug`                            |
| Сборка                            | `cmake --build out/build/windows-x64-debug`                   |
| Конфигурация (Linux Release)      | `cmake .. -DCMAKE_BUILD_TYPE=Release` (в папке build)         |
| Компиляция (Linux)                | `make -j$(nproc)`                                             |
