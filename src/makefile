OUT = axon
VERSION ?= 3.1.0
DATADIR ?= /usr/share

DEFINES := -DVERSION=\"${VERSION}\" -DDATADIR=\"${DATADIR}\"

CC = gcc
PKGDEPS = gtk+-2.0 vte
CFLAGS := -Wall $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
LDFLAGS := $(shell pkg-config --libs ${PKGDEPS})

all: axon

axon: axon.c
	$(CC) $(CFLAGS) $(LDFLAGS) axon.c -o ${OUT}

clean:
	@${RM} ${OUT}

strip: all
	@strip --strip-all ${OUT}

debug: CFLAGS := -g -Wall $(shell pkg-config --cflags ${PKGDEPS}) ${DEFINES}
debug: clean all

.PHONY: all clean debug strip
