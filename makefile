# axon - lightweight terminal emulator
# See LICENSE file for license details

export # Export all variables to sub-makes

VERSION = 0.0
OUT = axon

DISTFILES = LICENSE makefile README TODO colorschemes/ data/ src/

PREFIX ?= /usr
MANPREFIX ?= ${PREFIX}/share/man
DATADIR ?= ${PREFIX}/share

all: axon

axon:
	@${MAKE} -C src/
# For convenience, create link to executable
	@ln -sf src/axon axon

manpage:
	@echo generating manpage
	@sed -r -i "s/\"[0-9]{4}-[0-9]{2}-[0-9]{2}\"/\"$(shell date +%Y-%m-%d)\"/g" data/axon.1
	@sed -r -i "s/axon\\\-([0-9]*\.[0-9]*)*/axon\\\-${VERSION}/g" data/axon.1

clean:
	@echo cleaning
	@${RM} ${OUT} axon-${VERSION}.tar.gz
	@${MAKE} -C src/ clean

strip: all
	@echo striping executable
	@${MAKE} -C src/ strip

debug:
	@echo making debug build
	@${MAKE} -C src/ debug

dist: clean
	@echo creating dist tarball
	@mkdir axon-${VERSION}/
	@cp -r ${DISTFILES} axon-${VERSION}/
	@tar -czf axon-${VERSION}.tar.gz axon-${VERSION}/
	@rm -rf axon-${VERSION}/

install: all manpage
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@install -D -m755 src/axon ${DESTDIR}${PREFIX}/bin/axon
	@echo installing desktop file to ${DESTDIR}${DATADIR}/applications
	@install -D -m644 data/axon.desktop ${DESTDIR}${DATADIR}/applications/axon.desktop
	@echo installing colorscheme files to ${DESTDIR}${DATADIR}/axon
	@mkdir -p ${DESTDIR}${DATADIR}/axon/
	@cp -rf colorschemes/ ${DESTDIR}${DATADIR}/axon/
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@install -D -m644 data/axon.1 ${DESTDIR}${MANPREFIX}/man1/axon.1
	@echo installing example configuration file to ${DESTDIR}${PREFIX}/share/doc/axon/axonrc
	@install -D -m644 data/axonrc ${DESTDIR}${PREFIX}/share/doc/axon/axonrc
	@echo run "make uninstall" to remove

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/axon
	@echo removing desktop file from ${DESTDIR}${DATADIR}/applications
	@rm -f ${DESTDIR}${DATADIR}/applications/axon.desktop
	@echo removing colorscheme files from ${DESTDIR}${DATADIR}/axon
	@rm -rf ${DESTDIR}${DATADIR}/axon/colorschemes/
	@echo removing program data directory
	@rm -rf ${DATADIR}/axon
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/axon.1
	@echo removing example configuration file from ${DESTDIR}${PREFIX}/share/doc/axon/axonrc
	@rm -f ${DESTDIR}${PREFIX}/share/doc/axon/axonrc
	@echo removing program documentation directory
	@rm -rf ${DESTDIR}${PREFIX}/share/doc/axon/

.PHONY: all axon clean debug dist install uninstall
