# sinit version
VERSION = 0.4

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

CC = cc
LD = $(CC)
CPPFLAGS =
CFLAGS   = -Wall -Os $(CPPFLAGS)
LDFLAGS  = -static
