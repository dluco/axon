OUT = axon
VERSION ?= 2.0.0
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

config.o: config.c
	@echo $(CC) -c config.c
	@$(CC) $(CFLAGS) config.c

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
	@${RM} ${OUT} ${OBJ}

strip: all
	@strip --strip-all ${OUT}

debug: CFLAGS := -c -g -Wall $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
debug: clean all

.PHONY: all clean debug strip
