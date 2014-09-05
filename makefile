# axon - lightweight terminal emulator
# See LICENSE file for license details

export # Export all variables to sub-makes

PACKAGE=axon
VERSION=0.0

DATADIR=/usr/share

all: axon

axon:
	@${MAKE} -C src/
# For convenience, create link to executable
	@ln -sf src/${PACKAGE} ${PACKAGE}

clean:
	@echo cleaning
	@rm -rf ${PACKAGE} ${PACKAGE}-${VERSION}/ ${PACKAGE}-${VERSION}.tar.gz
	@${MAKE} -C src/ clean

debug:
	@echo making debug build
	@${MAKE} -C src/ debug

dist: clean
	@echo creating dist tarball
	@mkdir -p ${PACKAGE}-${VERSION}/
	@cp -r AUTHORS LICENSE makefile README TODO \
		colorschemes/ data/ src/ ${PACKAGE}-${VERSION}/
	@tar -czf ${PACKAGE}-${VERSION}.tar.gz ${PACKAGE}-${VERSION}/
	@rm -rf ${PACKAGE}-${VERSION}/

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f src/${PACKAGE} ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/${PACKAGE}
	@echo installing desktop file to ${DESTDIR}${DATADIR}/applications
	@mkdir -p ${DESTDIR}${DATADIR}/applications
	@cp -f data/${PACKAGE}.desktop ${DESTDIR}${DATADIR}/applications/
	@echo installing colorscheme files to ${DESTDIR}${DATADIR}/${PACKAGE}
	@mkdir -p ${DESTDIR}${DATADIR}/${PACKAGE}
	@cp -rf colorschemes/ ${DESTDIR}${DATADIR}/${PACKAGE}/
	@echo run "make uninstall" to remove

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/${PACKAGE}
	@echo removing desktop file from ${DESTDIR}${DATADIR}/applications
	@rm -f ${DESTDIR}${DATADIR}/applications/${PACKAGE}.desktop
	@echo removing colorscheme files from ${DESTDIR}${DATADIR}/${PACKAGE}
	@rm -rf ${DESTDIR}${DATADIR}/${PACKAGE}/colorschemes/
	@echo removing program data dir
	@rm -rf ${DATADIR}/${PACKAGE}

.PHONY: all axon clean debug dist install uninstall
