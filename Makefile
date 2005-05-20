NAME := libsimpleparser
VERSION := $(shell grep '^Version:' $(NAME).spec | sed 's/[^0-9]*\([0-9\.]*\)[^0-9]*$$/\1/')
RPMDIR := $(shell rpm --eval %{_topdir})

CC=gcc
#CFLAGS=-ggdb -DDEBUG
#LDFLAGS=-lefence
CFLAGS=
LDFLAGS=

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

rpm: clean
	cp -f $(NAME).spec $(RPMDIR)/SPECS
	cd .. && EXCLUDES=`find $(NAME) -name ".*" -mindepth 1 | sed 's/^/--exclude\n/' | xargs` \
	&& tar zcvf $(RPMDIR)/SOURCES/$(NAME)-$(VERSION).tar.gz $(NAME) $$EXCLUDES
	rpmbuild -ba $(RPMDIR)/SPECS/$(NAME).spec
