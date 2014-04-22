include config.mk

.POSIX:
.SUFFIXES: .c .o

SRC = sinit.c

OBJ = $(SRC:.c=.o)
BIN = $(SRC:.c=)

all: options bin

options:
	@echo sinit build options:
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CC       = $(CC)"

bin: $(BIN)

$(OBJ): config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

.o:
	@echo LD $@
	@$(LD) -o $@ $< $(LDFLAGS)

.c.o:
	@echo CC $<
	@$(CC) -c -o $@ $< $(CFLAGS)

dist: clean
	@echo creating dist tarball
	@mkdir -p sinit-$(VERSION)
	@cp LICENSE Makefile README config.def.h config.mk sinit.8 sinit.c \
		sinit-$(VERSION)
	@tar -cf sinit-$(VERSION).tar sinit-$(VERSION)
	@gzip sinit-$(VERSION).tar
	@rm -rf sinit-$(VERSION)

install: all
	@echo installing executable to $(DESTDIR)$(PREFIX)/bin
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	@chmod 755 $(DESTDIR)$(PREFIX)/bin/$(BIN)
	@echo installing manual page to $(DESTDIR)$(MANPREFIX)/man8
	@mkdir -p $(DESTDIR)$(MANPREFIX)/man8
	@sed "s/VERSION/$(VERSION)/g" < sinit.8 > $(DESTDIR)$(MANPREFIX)/man8/sinit.8
	@chmod 644 $(DESTDIR)$(MANPREFIX)/man8/sinit.8

uninstall:
	@echo removing executable from $(DESTDIR)$(PREFIX)/bin
	@cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN)
	@echo removing manual page from $(DESTDIR)$(MANPREFIX)/man8
	@rm -f $(DESTDIR)$(MANPREFIX)/man8/sinit.8

clean:
	@echo cleaning
	@rm -f $(BIN) $(OBJ) sinit-$(VERSION).tar.gz
