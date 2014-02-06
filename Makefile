include config.mk

.POSIX:
.SUFFIXES: .c .o

LIB = \
	util/eprintf.o

SRC = sinit.c

OBJ = $(SRC:.c=.o) $(LIB)
BIN = $(SRC:.c=)

all: options binlib

options:
	@echo sinit build options:
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CC       = $(CC)"

binlib: util.a
	$(MAKE) bin

bin: $(BIN)

$(OBJ): config.h util.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

.o:
	@echo LD $@
	@$(LD) -o $@ $< util.a $(LDFLAGS)

.c.o:
	@echo CC $<
	@$(CC) -c -o $@ $< $(CFLAGS)

util.a: $(LIB)
	@echo AR $@
	@$(AR) -r -c $@ $(LIB)
	@ranlib $@

install: all
	@echo installing executable to $(DESTDIR)$(PREFIX)/bin
	@cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	@cd $(DESTDIR)$(PREFIX)/bin && chmod 755 $(BIN)

uninstall:
	@echo removing executable from $(DESTDIR)$(PREFIX)/bin
	@cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN)

clean:
	@echo cleaning
	@rm -f $(BIN) $(OBJ) $(LIB) util.a
