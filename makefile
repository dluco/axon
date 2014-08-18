PACKAGE=myterm
VERSION=0.0

DEFINES=-DVERSION=\"${VERSION}\" -DPACKAGE=\"${PACKAGE}\"


CC=gcc
PKGDEPS=gtk+-2.0 vte
CFLAGS=-c -Wall -O2 -flto $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
LDFLAGS=-O2 -flto $(shell pkg-config --libs ${PKGDEPS})

OBJ=callback.o config.o dialog.o main.o menu.o options.o terminal.o utils.o

all: myterm

debug: CFLAGS=-c -g -Wall $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
debug: clean all

myterm: $(OBJ)
	@echo $(CC) -o myterm
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
	@rm -rf *.o myterm

install: all
	@echo installing
	@cp myterm.desktop /usr/share/applications/
	@echo run "make uninstall" to remove

uninstall:
	@echo uninstalling...
	@rm -rf ~/.config/myterm/
	@rm -f /usr/share/applications/myterm.desktop

.PHONY: all clean debug again
