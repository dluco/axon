pkgname=axon
pkgver=0.0
pkgrel=1
pkgdesc='A fast and lightweight terminal emulator built with GTK and VTE'
arch=('i686' 'x86_64')
url='http://github.com/dluco/axon'
license=('MIT')
depends=('vte')
makedepends=('pkgconfig')
source=("http://github.com/dluco/${pkgname}/archive/v${pkgver}.tar.gz")
md5sums=('39cb9b03056dd600fbcef1d50660f44e')

build() {
  cd "${srcdir}/${pkgname}-${pkgver}"

  make
}

package() {
  cd "${srcdir}/${pkgname}-${pkgver}"

  make PREFIX=/usr DESTDIR="${pkgdir}" install
  install -Dm0644 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
}