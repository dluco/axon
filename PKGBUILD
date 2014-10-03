pkgname=axon
pkgver=3.1.0
pkgrel=1
pkgdesc='A fast and lightweight terminal emulator built with GTK and VTE'
arch=('i686' 'x86_64')
url='http://github.com/dluco/axon'
license=('MIT')
depends=('vte')
makedepends=('pkgconfig')
source=("http://github.com/dluco/${pkgname}/archive/v${pkgver}.tar.gz")
md5sums=('f118cf1e1ae326352adca0864a781523')

build() {
  cd "${srcdir}/${pkgname}-${pkgver}"

  make
}

package() {
  cd "${srcdir}/${pkgname}-${pkgver}"

  make PREFIX=/usr DESTDIR="${pkgdir}" install
  install -Dm0644 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
}

# vim: ft=sh syn=sh
