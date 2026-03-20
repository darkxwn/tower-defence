# 🏰 Codename: Tower Defence

![C++](https://img.shields.io/badge/C++-17%2F20-blue?style=flat-square&logo=c%2B%2B)
![SFML](https://img.shields.io/badge/SFML-3.0.2-green?style=flat-square)
![Visual Studio 2022](https://img.shields.io/badge/Visual_Studio-2022-purple?style=flat-square&logo=visual-studio)

**Codename: Tower Defence** — это классическая игра в жанре «Башенная защита» (Tower Defense) с сеточной картой и волнами противников. 

🎓 *Проект разрабатывается в рамках курсовой работы студентов 2-го курса факультета ИСТ ДонНТУ.*

---

## 👥 Команда разработчиков
* **[@darkxwn](https://github.com/darkxwn)** — PLACEHOLDER_ROLE
* **[@BillVLad](https://github.com/BillVLad)** — PLACEHOLDER_ROLE
* **[Имя/Ник третьего участника]** — PLACEHOLDER_ROLE

---

## 🛠 Технологии
* **Язык:** C++ 
* **Графическая библиотека:** SFML 3

---

## ⚙️ Требования для запуска
Для работы с проектом и его сборки вам понадобится:
- Операционная система: **Windows 10 / 11**, Linux
- Среда разработки: **Visual Studio 2022** и выше (С установленным компонентом *«Разработка классических приложений на C++»*)
- Git (для клонирования репозитория)

---

## ⚙️ Сборка проекта

### Windows 
В Visual Studio достаточно собрать конфигурацию CMakeLists.txt, дождаться подготовки и выбрать в Target'е `tower-defence.exe`, после чего, проект соберётся в `.exe` файл.
### Linux
#### Arch Linux
1. Установка зависимостей:
```bash
sudo pacman -S --needed base-devel cmake git freetype2 libx11 libxrandr libxcursor libxi mesa glu openal libvorbis flac wayland libxkbcommon
```
2. Создание папки для сборки:
```bash
mkdir build
cd build
```
3. Подготовка проекта
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```
4. Компиляция
```bash
# make -j$(nproc) для быстрой сборки
make
```
Готово! Запустите `./tower-defence`.