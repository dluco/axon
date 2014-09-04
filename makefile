# axon - lightweight terminal emulator
# See LICENSE file for license details

export # Export all variables to sub-makes

PACKAGE=axon
VERSION=0.0

DATADIR=/usr/share

all: axon

axon:
	@${MAKE} -C src/

clean:
	@echo cleaning
	@rm -rf *.o ${PACKAGE}
	@${MAKE} -C src/ clean

debug:
	@echo making debug build
	@${MAKE} -C src/ debug

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f src/${PACKAGE} ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/${PACKAGE}
	@echo installing desktop file to ${DATADIR}/applications
	@mkdir -p ${DATADIR}/applications
	@cp -f data/${PACKAGE}.desktop ${DATADIR}/applications/
	@echo installing colorscheme files to ${DATADIR}/${PACKAGE}
	@mkdir -p ${DATADIR}/${PACKAGE}
	@cp -rf colorschemes/ ${DATADIR}/${PACKAGE}/
	@echo run "make uninstall" to remove

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/${PACKAGE}
	@echo removing desktop file from ${DATADIR}/applications
	@rm -f ${DATADIR}/applications/${PACKAGE}.desktop
	@echo removing colorscheme files from ${DATADIR}/${PACKAGE}
	@rm -rf ${DATADIR}/${PACKAGE}/colorschemes/
	@echo removing program data dir
	@rm -rf ${DATADIR}/${PACKAGE}

.PHONY: all clean debug install uninstall
