# Инструкция по сборке Codename: Tower Defence

Проект использует систему сборки **CMake** версии 3.15+. Все зависимости (SFML 3, nlohmann_json) загружаются автоматически во время конфигурации проекта.

## 🪟 Windows (Visual Studio)
1. Установите **Visual Studio 2022** или новее.
2. При установке выберите рабочую нагрузку **«Разработка классических приложений на C++»**.
3. Откройте папку проекта через: `Файл -> Открыть -> Локальная папка`.
4. Выберите конфигурацию `windows-x64-debug` (или release).
5. Дождитесь окончания генерации кэша CMake.
6. Выберите целью запуска `tower-defence.exe` и нажмите **F5**.

## 🐧 Linux
### 1. Установка зависимостей
Выберите команду для вашего дистрибутива:

**Arch Linux:**
```bash
sudo pacman -S --needed base-devel cmake git libdecor freetype2 libx11 libxrandr libxcursor libxi mesa glu openal libvorbis flac wayland wayland-protocols libxkbcommon
```
**Ubuntu / Debian / Astra Linux:**
```bash
sudo apt update && sudo apt install -y build-essential cmake git libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev libglu1-mesa-dev libfreetype-dev libopenal-dev libogg-dev libvorbis-dev libflac-dev libwayland-dev libxkbcommon-dev wayland-protocols libdecor-0-dev
```
**Fedora**
```bash
sudo dnf install -y gcc-c++ cmake make git libX11-devel libXrandr-devel libXcursor-devel libXi-devel systemd-devel mesa-libGL-devel mesa-libGLU-devel freetype-devel openal-soft-devel libogg-devel libvorbis-devel flac-devel wayland-devel libxkbcommon-devel libdecor-devel
```
### 2. Сборка
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./tower-defence
```
## 🍎 macOS
1. Установите Xcode Command Line Tools (xcode-select --install).
2. Установите Homebrew и зависимости:
```bash
brew install cmake flac libogg libvorbis openal-soft
```
3. Сборка:
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
Будет создан пакет Tower Defence.app.

## 🤖 Android
Сборка осуществляется через Gradle Wrapper. Требуется установленный Android SDK и NDK.
1. Настройте пути в android/local.properties:
```properties
sdk.dir=/path/to/android-sdk
ndk.dir=/path/to/android-ndk
```
2. Сборка APK из терминала:
```bash
cd android
./gradlew assembleDebug
```
Готовый файл появится в android/app/build/outputs/apk/debug/.
