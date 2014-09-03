PACKAGE=axon
VERSION=0.0

DATADIR=/usr/share

DEFINES=-DVERSION=\"${VERSION}\" -DPACKAGE=\"${PACKAGE}\" \
	-DDATADIR=\"${DATADIR}\"

CC=gcc
PKGDEPS=gtk+-2.0 vte
CFLAGS=-c -Wall -O2 -flto $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
LDFLAGS=-O2 -flto $(shell pkg-config --libs ${PKGDEPS})

OBJ=callback.o config.o dialog.o main.o menu.o options.o terminal.o utils.o

all: axon

debug: CFLAGS=-c -g -Wall $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
debug: clean all

axon: $(OBJ)
	@echo $(CC) -o ${PACKAGE}
	@$(CC) $(OBJ) $(LDFLAGS) -o ${PACKAGE}

callback.o: callback.c
	@echo $(CC) -c callback.c
	@$(CC) $(CFLAGS) callback.c

config.o: config.c
	@echo $(CC) -c config.c
	@$(CC) $(CFLAGS) config.c

dialog.o: dialog.c
	@echo $(CC) -c dialog.c
	@$(CC) $(CFLAGS) dialog.c

main.o: main.c
	@echo $(CC) -c main.c
	@$(CC) $(CFLAGS) main.c

menu.o: menu.c
	@echo $(CC) -c menu.c
	@$(CC) $(CFLAGS) menu.c

options.o: options.c
	@echo $(CC) -c options.c
	@$(CC) $(CFLAGS) options.c

terminal.o: terminal.c
	@echo $(CC) -c terminal.c
	@$(CC) $(CFLAGS) terminal.c

utils.o: utils.c
	@echo $(CC) -c utils.c
	@$(CC) $(CFLAGS) utils.c

clean:
	@echo cleaning...
	@rm -rf *.o ${PACKAGE}

install: all
	@echo installing...
	@cp data/${PACKAGE}.desktop /usr/share/applications/
	@cp -r colorschemes/ /usr/share/${PACKAGE}/
	@echo run "make uninstall" to remove

uninstall:
	@echo uninstalling...
	@rm -f /usr/share/applications/${PACKAGE}.desktop
	@rm -rf /usr/share/${PACKAGE}/colorschemes/

.PHONY: all clean debug install uninstall
