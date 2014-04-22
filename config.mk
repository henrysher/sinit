# sinit version
VERSION = 0.9

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC = cc
LD = $(CC)
CPPFLAGS =
CFLAGS   = -Wextra -Wall -Wno-unused-result -Os $(CPPFLAGS)
LDFLAGS  = -static
