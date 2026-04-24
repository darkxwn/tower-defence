# 🏰 Codename: Tower Defence

![C++](https://img.shields.io/badge/C++-20-blue?style=flat-square&logo=c%2B%2B)
![SFML](https://img.shields.io/badge/SFML-3.0.2-green?style=flat-square)
![Platforms](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Android-orange?style=flat-square)
![GitHub Actions](https://img.shields.io/badge/CI/CD-GitHub%20Actions-white?style=flat-square&logo=github-actions)

**Codename: Tower Defence** — это кроссплатформенная игра в жанре «Башенная защита» (Tower Defense), разработанная на современном C++ с использованием библиотеки SFML 3.

🎓 *Проект разрабатывается в рамках курсовой работы студентов 2-го курса факультета ИСТ ДонНТУ.*

---

## 🚀 Особенности
- **Кроссплатформенность:** Полная поддержка Windows, Linux, macOS и Android из единого кодовой базы.
- **Движок:** Собственная реализация системы камер с поддержкой зума и панорамирования.
- **Управление:** Адаптивный ввод (Мышь + Сенсорный экран с поддержкой жестов).
- **Архитектура:** Использование современных стандартов C++20 и автоматизированная сборка через CMake.
- **Сохранения:** Система прогресса на базе JSON.

---

## 🛠 Технологии
- **Язык:** C++20
- **Графика и Ввод:** SFML 3.0.2
- **Сборка:** CMake + Ninja
- **Управление зависимостями:** FetchContent (автоматическая загрузка SFML и nlohmann_json)
- **CI/CD:** GitHub Actions (автоматическая сборка артефактов под все ОС)

---

## 👥 Команда разработчиков
* **[@darkxwn](https://github.com/darkxwn)** — Игровая логика, UI, архитектура.
* **[@BillVLad](https://github.com/BillVLad)** — Кроссплатформенная интеграция, Android NDK, CI/CD.

---

## ⚙️ Сборка и запуск
Подробные инструкции по установке зависимостей и компиляции под разные операционные системы находятся в отдельном файле:

👉 **[Инструкция по сборке (BUILDING.md)](BUILDING.md)**

---

## 📄 Лицензия
Проект распространяется под лицензией GNU GPL. Подробности в файле [LICENSE](LICENSE).
