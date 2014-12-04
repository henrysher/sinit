include config.mk

OBJ = sinit.o
BIN = sinit

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

$(OBJ): config.h

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man8
	sed "s/VERSION/$(VERSION)/g" < $(BIN).8 > $(DESTDIR)$(MANPREFIX)/man8/$(BIN).8

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)
	rm -f $(DESTDIR)$(MANPREFIX)/man8/$(BIN).8

dist: clean
	mkdir -p sinit-$(VERSION)
	cp LICENSE Makefile README config.def.h config.mk sinit.8 sinit.c sinit-$(VERSION)
	tar -cf sinit-$(VERSION).tar sinit-$(VERSION)
	gzip sinit-$(VERSION).tar
	rm -rf sinit-$(VERSION)

clean:
	rm -f $(BIN) $(OBJ) sinit-$(VERSION).tar.gz

.SUFFIXES: .def.h

.def.h.h:
	cp $< $@

.PHONY:
	all install uninstall dist clean
