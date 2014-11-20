pkgname=axon-git
_gitname=axon
pkgver=3.1.0.4.g772389a
pkgrel=1
pkgdesc='A fast and lightweight terminal emulator built with GTK and VTE'
arch=('i686' 'x86_64')
url='http://github.com/dluco/axon'
license=('MIT')
makedepends=('git')
provides=('axon')
conflicts=('axon')
depends=('vte')
source=("git://github.com/dluco/axon.git")
md5sums=('SKIP')

pkgver() {
  cd "$srcdir/$_gitname"
  git describe --tags | sed -e 	's:v::' -e 's/-/./g'
}

build() {
  cd "$srcdir/$_gitname"

  make PREFIX=/usr
}

package() {
  cd "$srcdir/$_gitname"

  make PREFIX=/usr DESTDIR="$pkgdir" install
  install -Dm0644 LICENSE "$pkgdir/usr/share/licenses/$_gitname/LICENSE"
}

# vim: ft=sh syn=sh
