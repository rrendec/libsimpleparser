CC=gcc
#CFLAGS=-ggdb -DDEBUG
#LDFLAGS=-lefence
CFLAGS=
LDFLAGS=

MAJOR=0
MINOR=1
REVISION=1
VERSION=$(MAJOR).$(MINOR).$(REVISION)

.PHONY: all clean install

all: test libsimpleparser.so.$(VERSION)

test: test.o simpleparser.o
	$(CC) $(LDFLAGS) -o test test.o simpleparser.o

test.o: test.c simpleparser.h
	$(CC) $(CFLAGS) -o test.o -c test.c

simpleparser.o: simpleparser.c simpleparser.h
	$(CC) $(CFLAGS) -o simpleparser.o -c simpleparser.c

libsimpleparser.so.$(VERSION): simpleparser.o
	$(CC) -s -shared -Wl,-soname,libsimpleparser.so simpleparser.o -o libsimpleparser.so.$(VERSION)

install: libsimpleparser.so.$(VERSION)
	cp -f simpleparser.h /usr/include
	chmod 644 /usr/include/simpleparser.h
	cp -f libsimpleparser.so.$(VERSION) /usr/lib
	chmod 755 /usr/lib/libsimpleparser.so.$(VERSION)
	ldconfig

clean:
	rm -f *~ *.o libsimpleparser.so.$(VERSION) test
