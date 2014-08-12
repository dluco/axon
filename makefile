VERSION=0.0

PACKAGE=myte

CC=gcc
CFLAGS=-c -Wall -O2 -flto $(shell pkg-config --cflags gtk+-2.0 vte) -DVERSION=\"${VERSION}\" -DPACKAGE=\"${PACKAGE}\"
LDFLAGS=-O2 -flto $(shell pkg-config --libs gtk+-2.0 vte)

OBJ=callback.o main.o terminal.o utils.o

all: myte

debug: CFLAGS=-c -g -Wall $(shell pkg-config --cflags gtk+-2.0 vte)
debug: all

myte: $(OBJ)
	@echo $(CC) -o myte
	@$(CC) $(OBJ) $(LDFLAGS) -o myte

callback.o: callback.c callback.h
	@echo $(CC) -c callback.c
	@$(CC) $(CFLAGS) callback.c

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
	rm -rf *.o myte

.PHONY: all clean debug
