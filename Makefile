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
	@cp LICENSE Makefile README config.def.h config.mk sinit.c \
		sinit-$(VERSION)
	@tar -cf sinit-$(VERSION).tar sinit-$(VERSION)
	@gzip sinit-$(VERSION).tar
	@rm -rf sinit-$(VERSION)

install: all
	@echo installing executable to $(DESTDIR)$(PREFIX)/bin
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	@cd $(DESTDIR)$(PREFIX)/bin && chmod 755 $(BIN)

uninstall:
	@echo removing executable from $(DESTDIR)$(PREFIX)/bin
	@cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN)

clean:
	@echo cleaning
	@rm -f $(BIN) $(OBJ) sinit-$(VERSION).tar.gz
