# sinit version
VERSION = 0.6

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

CC = cc
LD = $(CC)
CPPFLAGS =
CFLAGS   = -Wall -Os $(CPPFLAGS)
LDFLAGS  = -static
