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
	install -m 0644 -D simpleparser.h /usr/include/simpleparser.h 
	install -m 0755 -D libsimpleparser.so.$(VERSION) /usr/lib/libsimpleparser.so.$(VERSION)
	ldconfig

clean:
	rm -f *~ *.o libsimpleparser.so.$(VERSION) test
