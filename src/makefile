OUT = axon
VERSION ?= 1.0
DATADIR ?= /usr/share

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

DEFINES := -DVERSION=\"${VERSION}\" -DDATADIR=\"${DATADIR}\"

CC = gcc
PKGDEPS = gtk+-2.0 vte
CFLAGS := -c -Wall -O2 -flto $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
LDFLAGS := -O2 -flto $(shell pkg-config --libs ${PKGDEPS})

all: axon

axon: $(OBJ)
	@echo $(CC) -o ${OUT}
	@$(CC) $(OBJ) $(LDFLAGS) -o ${OUT}

callback.o: callback.c
	@echo $(CC) -c callback.c
	@$(CC) $(CFLAGS) callback.c

color.o: color.c
	@echo $(CC) -c color.c
	@$(CC) $(CFLAGS) color.c

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

search.o: search.c
	@echo $(CC) -c search.c
	@$(CC) $(CFLAGS) search.c

utils.o: utils.c
	@echo $(CC) -c utils.c
	@$(CC) $(CFLAGS) utils.c

clean:
	@${RM} ${OUT} ${OBJ}

strip: all
	@strip --strip-all ${OUT}

debug: CFLAGS := -c -g -Wall $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
debug: clean all

.PHONY: all clean debug strip
