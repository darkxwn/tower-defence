# Maintainer: billvlad
pkgname=tower-defence-git
pkgver=0.7.3
pkgrel=1
pkgdesc="Tower Defence Game (Project: Gyurza) built with SFML"
arch=('x86_64')
url="https://github.com/darkxwn/tower-defence"
license=('GPL3')

depends=('openal' 'libvorbis' 'flac' 'freetype2' 'libxrandr' 'systemd-libs', 'sfml', 'nlohmann-json')
makedepends=('cmake' 'git')

build() {
    # Сборка из локальных исходников проекта
    cmake -B build -S "$startdir" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build
}

package() {
    # Установка во временную директорию пакета
    cmake --install build --prefix "$pkgdir"
}