pkgname=axon
pkgver=1.0
pkgrel=1
pkgdesc='A fast and lightweight terminal emulator built with GTK and VTE'
arch=('i686' 'x86_64')
url='http://github.com/dluco/axon'
license=('MIT')
depends=('vte')
makedepends=('pkgconfig')
source=("http://github.com/dluco/${pkgname}/archive/v${pkgver}.tar.gz")
md5sums=('ea7be4c22ef30f0317832197fbac20d4')

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
