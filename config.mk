# sinit version
VERSION = 0.8

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

CC = cc
LD = $(CC)
CPPFLAGS =
CFLAGS   = -Wextra -Wall -Wno-unused-result -Os $(CPPFLAGS)
LDFLAGS  = -static
