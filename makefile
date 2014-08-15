PACKAGE=myterm
VERSION=0.0

DEFINES=-DVERSION=\"${VERSION}\" -DPACKAGE=\"${PACKAGE}\"


CC=gcc
PKGDEPS=gtk+-2.0 vte
CFLAGS=-c -Wall -O2 -flto $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
LDFLAGS=-O2 -flto $(shell pkg-config --libs ${PKGDEPS})

OBJ=callback.o config.o main.o terminal.o utils.o

all: myterm

debug: CFLAGS=-c -g -Wall $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
debug: all

myterm: $(OBJ)
	@echo $(CC) -o myterm
	@$(CC) $(OBJ) $(LDFLAGS) -o myterm

callback.o: callback.c
	@echo $(CC) -c callback.c
	@$(CC) $(CFLAGS) callback.c

config.o: config.c
	@echo $(CC) -c config.c
	@$(CC) $(CFLAGS) config.c

main.o: main.c
	@echo $(CC) -c main.c
	@$(CC) $(CFLAGS) main.c

terminal.o: terminal.c
	@echo $(CC) -c terminal.c
	@$(CC) $(CFLAGS) terminal.c

utils.o: utils.c
	@echo $(CC) -c utils.c
	@$(CC) $(CFLAGS) utils.c

clean:
	rm -rf *.o myterm

again: clean all

test:
	@echo ${VERSION}

.PHONY: all clean debug again
