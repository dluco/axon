CC=gcc
CFLAGS+=-c -g -Wall $(shell pkg-config --cflags gtk+-2.0 vte)
LDFLAGS+=$(shell pkg-config --libs gtk+-2.0 vte)

OBJ=callback.o main.o

all: myte

myte: $(OBJ)
	@echo $(CC) -o myte
	@$(CC) $(OBJ) $(LDFLAGS) -o myte

callback.o: callback.c callback.h
	@echo $(CC) -c callback.c
	@$(CC) $(CFLAGS) callback.c

main.o: main.c config.h
	@echo $(CC) -c main.c
	@$(CC) $(CFLAGS) main.c

clean:
	rm -rf *.o myte

.PHONY: all clean
